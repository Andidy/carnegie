ID3D12Debug* debugController;

/* ------------------- RENDERING ------------------- */
#pragma region WIN32
typedef struct win32_WindowDimension
{
  i32 width;
  i32 height;
} win32_WindowDimension;

internal win32_WindowDimension win32_GetWindowDimension(HWND window)
{
  RECT clientRect;
  GetClientRect(window, &clientRect);

  win32_WindowDimension windowDimension;
  windowDimension.width = clientRect.right - clientRect.left;
  windowDimension.height = clientRect.bottom - clientRect.top;
  return windowDimension;
}
#pragma endregion

#pragma region D3D12

global f32 aspectRatio = 1200 / 900;
global u32 global_windowWidth = 1200;
global u32 global_windowHeight = 900;

struct Vertex
{
  f32 x, y, z;
  f32 r, g, b, a;
  f32 u, v;
};

struct ConstantBufferPerObject
{
  mat4 mvp;
};

// Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
// We are only able to read at 256 byte intervals from the start of a resource heap, so we will
// make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
// Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
// we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
// The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
// buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
// were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
// being copied.
i32 constantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

#define def_FrameCount 3
static const UINT frameCount = def_FrameCount;

// Pipeline objects
D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;

IDXGISwapChain3* swapchain;
ID3D12Device* device;

ID3D12CommandAllocator* commandAllocator[def_FrameCount];
ID3D12CommandQueue* commandQueue;
ID3D12GraphicsCommandList* commandList;

ID3D12Resource* renderTargets[def_FrameCount];
ID3D12DescriptorHeap* rtvHeap;
UINT rtvDescSize;


struct Renderer
{
  // --- Input Data for GPU ---
  // rootSig and pso should be made into arrays and expanded as neccesary
  ID3D12RootSignature* rootSignature;
  ID3D12PipelineState* pipelineStateObject;

  // synchronization objects
  UINT frameIndex;
  HANDLE fenceEvent;
  ID3D12Fence* fence[def_FrameCount];
  u64 fenceValue[def_FrameCount];
};
Renderer renderer;

// app resources
ID3D12Resource* vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
ID3D12Resource* indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;
ID3D12Resource* depthStencilBuffer;
ID3D12DescriptorHeap* dsDescHeap;

ConstantBufferPerObject cbPerObject;
ID3D12Resource* constantBufferUploadHeaps[frameCount];
u8* cbvGPUAddess[frameCount];

// camera ---
mat4 projmat;
mat4 viewmat;

vec3 cameraPos;
vec3 cameraTarget;
vec3 cameraUp;
// camera ---

// cube 1 ---
//XMFLOAT4 cube1pos;
// cube 1 ---

// cube 2 ---
//XMFLOAT4 cube2offset;
// cube 2 ---

i32 numCubeIndices;

ID3D12Resource* textureBuffer;
ID3D12DescriptorHeap* mainDescriptorHeap;
ID3D12Resource* textureBufferUploadHeap;



void WaitForPreviousFrame()
{
  hr = 0;

  // swap current rtv buffer index so we draw on the correct buffer
  renderer.frameIndex = swapchain->GetCurrentBackBufferIndex();

  // if the current fence value is less than "fenceValue"
  // then we know the GPU has not finished executing the command queue
  // since it has not reached the commandQueue->Signal() command
  if (renderer.fence[renderer.frameIndex]->GetCompletedValue() < renderer.fenceValue[renderer.frameIndex])
  {
    hr = renderer.fence[renderer.frameIndex]->SetEventOnCompletion(renderer.fenceValue[renderer.frameIndex], renderer.fenceEvent);
    win32_CheckSucceeded(hr);

    // we will wait until the fence has triggered the event to proceed
    WaitForSingleObject(renderer.fenceEvent, INFINITE);
  }

  // increment fence value for the next frame
  renderer.fenceValue[renderer.frameIndex]++;
}

void InitD3D(HWND window)
{
  /* Create the device */
  IDXGIFactory4* dxgiFactory;
  hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
  win32_CheckSucceeded(hr);

  IDXGIAdapter1* adapter;
  i32 adapterIndex = 0;
  b32 adapterFound = false;

  while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
  {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
    {
      adapterIndex++;
      continue;
    }

    hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), 0);
    if (SUCCEEDED(hr))
    {
      adapterFound = true;
      break;
    }
    adapterIndex++;
  }

  if (!adapterFound)
  {
    MessageBoxA(0, "No suitable GPU detected", "Adapter not found", MB_OK);
    __debugbreak();
  }

  hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
  win32_CheckSucceeded(hr);

  /* Create Render Target View Command Queue */
  D3D12_COMMAND_QUEUE_DESC commandQueueDesc = { 0 };
  commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  commandQueueDesc.NodeMask = 0;
  commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
  win32_CheckSucceeded(hr);

  /* Create the swapchain */
  DXGI_SWAP_CHAIN_DESC1 swapchainDesc = { 0 };
  swapchainDesc.Width = global_windowWidth;
  swapchainDesc.Height = global_windowHeight;
  swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapchainDesc.BufferCount = 3;
  swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapchainDesc.SampleDesc.Count = 1;
  swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
  swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
  swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

  IDXGISwapChain1* swapchain1;
  hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, window, &swapchainDesc, 0, 0, &swapchain1);
  win32_CheckSucceeded(hr);

  swapchain = (IDXGISwapChain3*)(swapchain1);
  renderer.frameIndex = swapchain->GetCurrentBackBufferIndex();

  /* Create the backbuffers (rtvs) Descriptor Heap */
  // Create Descriptor Heap
  {
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = { 0 };
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descHeapDesc.NumDescriptors = frameCount;
    hr = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&rtvHeap));
    win32_CheckSucceeded(hr);
    rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  // Create frame resources
  {
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (i32 i = 0; i < frameCount; i++)
    {
      // Create an rtv for each frame
      hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
      win32_CheckSucceeded(hr);

      device->CreateRenderTargetView(renderTargets[i], NULL, cpuDescriptorHandle);

      cpuDescriptorHandle.ptr = (SIZE_T)(((INT64)cpuDescriptorHandle.ptr) + ((INT64)1) * ((INT64)rtvDescSize));
    }
  }

  for (i32 i = 0; i < frameCount; i++)
  {
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
    win32_CheckSucceeded(hr);
  }

  // Create the command list
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], NULL, IID_PPV_ARGS(&commandList));
  win32_CheckSucceeded(hr);

  // Create Fences & Fence Events
  for (i32 i = 0; i < frameCount; i++)
  {
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&renderer.fence[i]));
    win32_CheckSucceeded(hr);
    renderer.fenceValue[i] = 0;
  }

  // Create an event handle to use for frame synchronization
  renderer.fenceEvent = CreateEvent(0, false, false, 0);
  if (renderer.fenceEvent == 0)
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
    win32_CheckSucceeded(hr);
  }





  // Create RootSignature
  // Create a root descriptor
  D3D12_ROOT_DESCRIPTOR rootCBVDescriptor = { 0 };
  rootCBVDescriptor.RegisterSpace = 0;
  rootCBVDescriptor.ShaderRegister = 0;

  // create a descriptor range (descriptor table) and fill it out
  // this is a range of descriptors inside a descriptor heap
  D3D12_DESCRIPTOR_RANGE descriptorTableRanges[1];
  descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  descriptorTableRanges[0].NumDescriptors = 1;
  descriptorTableRanges[0].BaseShaderRegister = 0;
  descriptorTableRanges[0].RegisterSpace = 0;
  descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

  // create a descriptor table
  D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
  descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges);
  descriptorTable.pDescriptorRanges = &descriptorTableRanges[0];

  // Create a root parameter
  D3D12_ROOT_PARAMETER rootParams[2];
  rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  rootParams[0].Descriptor = rootCBVDescriptor;
  rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

  rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  rootParams[1].DescriptorTable = descriptorTable;
  rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  // Create static sampler
  D3D12_STATIC_SAMPLER_DESC sampler = { 0 };
  sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.MipLODBias = 0;
  sampler.MaxAnisotropy = 0;
  sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
  sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
  sampler.MinLOD = 0.0f;
  sampler.MaxLOD = D3D12_FLOAT32_MAX;
  sampler.ShaderRegister = 0;
  sampler.RegisterSpace = 0;
  sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.NumParameters = _countof(rootParams);
  rootSignatureDesc.pParameters = rootParams;
  rootSignatureDesc.NumStaticSamplers = 1;
  rootSignatureDesc.pStaticSamplers = &sampler;
  rootSignatureDesc.Flags =
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
    | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
    | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

  ID3DBlob* signature;
  hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, 0);
  win32_CheckSucceeded(hr);

  hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&renderer.rootSignature));
  win32_CheckSucceeded(hr);

  // create vertex and pixel shaders

  // when debugging, we can compile the shader files at runtime.
  // but for release versions, we can compile the hlsl shaders
  // with fxc.exe to create .cso files, which contain the shader
  // bytecode. We can load the .cso files at runtime to get the
  // shader bytecode, which of course is faster than compiling
  // them at runtime

  // compile vertex shader
  ID3DBlob* vertexShader; // d3d blob for holding vertex shader bytecode
  ID3DBlob* errorBuff; // a buffer holding the error data if any
  hr = D3DCompileFromFile(L"../VertexShader.hlsl", 0, 0, "main", "vs_5_0",
    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &errorBuff);
  if (FAILED(hr))
  {
    OutputDebugStringA((char*)errorBuff->GetBufferPointer());
    __debugbreak();
  }

  // fill out a shader bytecode structure
  D3D12_SHADER_BYTECODE vertexShaderBytecode = { 0 };
  vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
  vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

  // compile pixel shader
  ID3DBlob* pixelShader;
  hr = D3DCompileFromFile(L"../PixelShader.hlsl", 0, 0, "main", "ps_5_0",
    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &errorBuff);
  if (FAILED(hr))
  {
    OutputDebugStringA((char*)errorBuff->GetBufferPointer());
    __debugbreak();
  }

  // fill out a shader bytecode structure
  D3D12_SHADER_BYTECODE pixelShaderBytecode = { 0 };
  pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
  pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

  // create input layout

  // The input layout is used by the Input Assembler so that it knows
  // how to read the vertex data bound to it.

  D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  // fill out an input layout description structure
  D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = { 0 };
  inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
  inputLayoutDesc.pInputElementDescs = inputLayout;

  // create a pipeline state object (PSO)

  // In a real application, you will have many pso's. for each different shader
  // or different combinations of shaders, different blend states or different rasterizer states,
  // different topology types (point, line, triangle, patch), or a different number
  // of render targets you will need a pso

  // VS is the only required shader for a pso. You might be wondering when a case would be where
  // you only set the VS. It's possible that you have a pso that only outputs data with the stream
  // output, and not on a render target, which means you would not need anything after the stream
  // output.

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
  psoDesc.InputLayout = inputLayoutDesc;
  psoDesc.pRootSignature = renderer.rootSignature;
  psoDesc.VS = vertexShaderBytecode;
  psoDesc.PS = pixelShaderBytecode;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  psoDesc.SampleDesc.Count = 1;
  psoDesc.SampleMask = 0xffffffff;
  psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
  psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.NumRenderTargets = 1;
  hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&renderer.pipelineStateObject));
  win32_CheckSucceeded(hr);

  // Create vertex buffer
  Vertex vList[] = {
    // front face
    { -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
    {  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f },
    { -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f },

    // right side face
    {  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 0.0f },
    {  0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f,  0.0f, 0.0f },

    // left side face
    { -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
    { -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f },
    { -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f },
    { -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f },

    // back face
    {  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
    { -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f },
    {  0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f },
    { -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f },

    // top face
    { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 0.0f },
    {  0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f },
    { -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,  0.0f, 0.0f },

    // bottom face
    {  0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
    { -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f },
    {  0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f },
    { -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f }
  };
  i32 vBufferSize = sizeof(vList);

  // create default heap
  // default heap is memory on the GPU. Only the GPU has access to this memory
  // To get data into this heap, we will have to upload the data using
  // an upload heap
  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
    D3D12_RESOURCE_STATE_COPY_DEST,
    0,
    IID_PPV_ARGS(&vertexBuffer)
  );
  win32_CheckSucceeded(hr);

  // give the heap a name for debugging
  vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

  // create upload heap
  ID3D12Resource* vBufferUploadHeap;
  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    0, IID_PPV_ARGS(&vBufferUploadHeap)
  );
  win32_CheckSucceeded(hr);
  vBufferUploadHeap->SetName(L"VertexBufferUploadResourceHeap");

  // store buffer in upload heap
  D3D12_SUBRESOURCE_DATA vertexData = { 0 };
  vertexData.pData = (BYTE*)(vList);
  vertexData.RowPitch = vBufferSize;
  vertexData.SlicePitch = vBufferSize;

  // we now create a command with a command list to copy the data from
  // the upload heap to the default heap
  UpdateSubresources(commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

  // transition the vertex buffer data from copy destination state to the vertex buffer state
  commandList->ResourceBarrier(
    1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
  );

  // create index buffer
  DWORD iList[] = {
    // front face
    0, 1, 2, // first triangle
    0, 3, 1, // second triangle

    // left face
    4, 5, 6, // first triangle
    4, 7, 5, // second triangle

    // right face
    8, 9, 10, // first triangle
    8, 11, 9, // second triangle

    // back face
    12, 13, 14, // first triangle
    12, 15, 13, // second triangle

    // top face
    16, 17, 18, // first triangle
    16, 19, 17, // second triangle

    // bottom face
    20, 21, 22, // first triangle
    20, 23, 21, // second triangle
  };
  i32 iBufferSize = sizeof(iList);

  numCubeIndices = sizeof(iList) / sizeof(DWORD);

  // create the default heap to hold index buffer
  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
    D3D12_RESOURCE_STATE_COPY_DEST,
    0, IID_PPV_ARGS(&indexBuffer)
  );
  win32_CheckSucceeded(hr);
  indexBuffer->SetName(L"IndexBufferResourceName");

  // create upload heap to upload index buffer
  ID3D12Resource* iBufferUploadHeap;
  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    0, IID_PPV_ARGS(&iBufferUploadHeap)
  );
  win32_CheckSucceeded(hr);
  iBufferUploadHeap->SetName(L"IndexBufferUploadResourceHeap");

  // store index buffer in upload heap
  D3D12_SUBRESOURCE_DATA indexData = { 0 };
  indexData.pData = (BYTE*)(iList);
  indexData.RowPitch = iBufferSize;
  indexData.SlicePitch = iBufferSize;

  UpdateSubresources(commandList, indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);

  commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

  // Depth stencil heap
  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = { 0 };
  dsvHeapDesc.NumDescriptors = 1;
  dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescHeap));
  win32_CheckSucceeded(hr);

  D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = { 0 };
  depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
  depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

  D3D12_CLEAR_VALUE depthOptimizedClearValue = { 0 };
  depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
  depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
  depthOptimizedClearValue.DepthStencil.Stencil = 0;

  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, global_windowWidth, global_windowHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &depthOptimizedClearValue,
    IID_PPV_ARGS(&depthStencilBuffer)
  );
  win32_CheckSucceeded(hr);
  dsDescHeap->SetName(L"Depth/StencilResourceHeap");

  device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescHeap->GetCPUDescriptorHandleForHeapStart());

  // create the constant buffer resource heap
  // We will update the constant buffer one or more times per frame, so we will use only an upload heap
  // unlike previously we used an upload heap to upload the vertex and index data, and then copied over
  // to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
  // efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
  // will be modified and uploaded at least once per frame, so we only use an upload heap

  // first we will create a resource heap (upload heap) for each frame for the cubes constant buffers
  // As you can see, we are allocating 64KB for each resource we create. Buffer resource heaps must be
  // an alignment of 64KB. We are creating 3 resources, one for each frame. Each constant buffer is 
  // only a 4x4 matrix of floats in this tutorial. So with a float being 4 bytes, we have 
  // 16 floats in one constant buffer, and we will store 2 constant buffers in each
  // heap, one for each cube, thats only 64x2 bits, or 128 bits we are using for each
  // resource, and each resource must be at least 64KB (65536 bits)

  for (i32 i = 0; i < frameCount; i++)
  {
    // create resource for cube 1
    hr = device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      0, IID_PPV_ARGS(&constantBufferUploadHeaps[i])
    );
    win32_CheckSucceeded(hr);
    constantBufferUploadHeaps[i]->SetName(L"ConstantBufferUploadHeap");

    ZeroMemory(&cbPerObject, sizeof(cbPerObject));

    CD3DX12_RANGE readRange(0, 0);
    hr = constantBufferUploadHeaps[i]->Map(0, &readRange, (void**)(&cbvGPUAddess[i]));
    win32_CheckSucceeded(hr);

    // Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
    // so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.

    // cube 1
    memcpy(cbvGPUAddess[i], &cbPerObject, sizeof(cbPerObject));
    // cube 2
    memcpy(cbvGPUAddess[i] + constantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
  }

  InitializeTextureFromFileName(L"cat.png", &textureBuffer, &textureBufferUploadHeap, &mainDescriptorHeap, device, commandList);

  // now we execute the command list to upload the initial assests (triangle data)
  commandList->Close();
  ID3D12CommandList* ppCommandLists[] = { commandList };
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // increment the fence value now, otherwise the buffer might not be uploaded by time we start drawing
  renderer.fenceValue[renderer.frameIndex]++;
  hr = commandQueue->Signal(renderer.fence[renderer.frameIndex], renderer.fenceValue[renderer.frameIndex]);
  win32_CheckSucceeded(hr);

  // done with image data so we can free it
  // free(imageData);

  // create a vertex buffer view for the triangle
  vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
  vertexBufferView.StrideInBytes = sizeof(Vertex);
  vertexBufferView.SizeInBytes = vBufferSize;

  indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
  indexBufferView.Format = DXGI_FORMAT_R32_UINT;
  indexBufferView.SizeInBytes = iBufferSize;

  // Fill out the Viewport
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = (f32)global_windowWidth;
  viewport.Height = (f32)global_windowHeight;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;

  // Fill out a scissor rect
  scissorRect.left = 0;
  scissorRect.top = 0;
  scissorRect.right = global_windowWidth;
  scissorRect.bottom = global_windowHeight;


  // build proj and view matrix
  projmat = PerspectiveMat(45.0f, aspectRatio, 0.1f, 1000.0f);

  // set starting camera state
  cameraPos = Vec3(0.0f, 5.0f, -5.0f);
  cameraTarget = Vec3(0.0f, 0.0f, 0.0f);
  cameraUp = Vec3(0.0f, 1.0f, 0.0f);

  // view matrix
  viewmat = LookAtMat(cameraPos, cameraTarget, cameraUp);

  OutputDebugStringA("Successfully Initialized D3D\n");
}

void UpdatePipeline()
{
  hr = 0;

  WaitForPreviousFrame();

  hr = commandAllocator[renderer.frameIndex]->Reset();
  win32_CheckSucceeded(hr);

  // reset the command list. by resetting the command list we are putting it into
  // a recording state so we can start recording commands into the command allocator.
  // the command allocator that we reference here may have multiple command lists
  // associated with it, but only one can be recording at any time. Make sure
  // that any other command lists associated to this command allocator are in
  // the closed state (not recording).
  // Here you will pass an initial pipeline state object as the second parameter,
  // but in this tutorial we are only clearing the rtv, and do not actually need
  // anything but an initial default pipeline, which is what we get by setting
  // the second parameter to NULL
  hr = commandList->Reset(commandAllocator[renderer.frameIndex], renderer.pipelineStateObject);
  win32_CheckSucceeded(hr);

  D3D12_RESOURCE_BARRIER resourceBarrier = { 0 };
  resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  resourceBarrier.Transition.pResource = renderTargets[renderer.frameIndex];
  resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  commandList->ResourceBarrier(1, &resourceBarrier);

  D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
  cpuDescriptorHandle.ptr = (SIZE_T)(((INT64)cpuDescriptorHandle.ptr) + ((INT64)renderer.frameIndex) * ((INT64)rtvDescSize));

  D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsDescHeap->GetCPUDescriptorHandleForHeapStart();

  commandList->OMSetRenderTargets(1, &cpuDescriptorHandle, false, &dsvHandle);
  const f32 clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  commandList->ClearRenderTargetView(cpuDescriptorHandle, clearColor, 0, 0);
  commandList->ClearDepthStencilView(dsDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, 0);

  commandList->SetGraphicsRootSignature(renderer.rootSignature);

  // set the descriptor heap
  ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap };
  commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  // set the descriptor table to the descriptor heap
  commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

  // draw triangle
  commandList->RSSetViewports(1, &viewport); // set the viewports
  commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
  commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
  commandList->IASetIndexBuffer(&indexBufferView);

  // first cube
  commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[renderer.frameIndex]->GetGPUVirtualAddress());
  commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

  // second cube
  commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[renderer.frameIndex]->GetGPUVirtualAddress() + constantBufferPerObjectAlignedSize);
  commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

  // transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
  // warning if present is called on the render target when it's not in the present state
  resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  resourceBarrier.Transition.pResource = renderTargets[renderer.frameIndex];
  resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  commandList->ResourceBarrier(1, &resourceBarrier);

  hr = commandList->Close();
  win32_CheckSucceeded(hr);
}

void Render()
{
  hr = 0;

  UpdatePipeline();

  // create an array of command lists (only 1 here)
  ID3D12CommandList* ppCommandLists[] = { commandList };

  // execute the array of command lists
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // this command goes in at the end of our command queue
  // we will know when our command queue had finished because
  // the fence value will be set to "fenceValue" variable from the GPU
  // since the command queue is being executed on the GPU
  hr = commandQueue->Signal(renderer.fence[renderer.frameIndex], renderer.fenceValue[renderer.frameIndex]);
  win32_CheckSucceeded(hr);

  // present the current back buffer
  hr = swapchain->Present(0, 0);
  win32_CheckSucceeded(hr);
}

void Update(HWND window, game_Memory* gameMemory)
{
  // build vp matrix ---
  // this can be used for all objects in the world

  // (handles window resize)
  win32_WindowDimension dim = win32_GetWindowDimension(window);
  global_windowWidth = dim.width;
  global_windowHeight = dim.height;
  aspectRatio = (f32)global_windowWidth / (f32)global_windowHeight;
  
  mat4 proj = PerspectiveMat(45.0f, aspectRatio, 0.1f, 1000.0f);
  projmat = proj;
  
  game_State* gameState = (game_State*)gameMemory->data;
  // load global view matrix to local var
  mat4 view = viewmat;
  mat4 vp = MulMat(proj, view);
  // build vp matrix ---

  // ------------ CUBE 1 --------------

  mat4 worldmat = TranslateMat(gameState->entities[0].pos);
  // build mvp matrix for cube 1 and send to gpu ---
  mat4 mvp = MulMat(vp, worldmat);

  cbPerObject.mvp = mvp;
  memcpy(cbvGPUAddess[renderer.frameIndex], &cbPerObject, sizeof(cbPerObject));

  // ------------ CUBE 2 -------------------
  // rotate, translate, and scale cube 2 ---

  mat4 t2mat = TranslateMat(gameState->entities[1].pos);
  mat4 rotmat = RotateMat(45.0f, Vec3(0, 1, 0));
  mat4 smat = ScaleMat({ 0.5f, 0.5f, 0.5f });

  worldmat = MulMat(rotmat, smat);
  worldmat = MulMat(t2mat, worldmat);
  //worldmat = t2mat;

  // build mvp matrix for cube 2 and send to gpu ---
  mvp = MulMat(vp, worldmat);
  cbPerObject.mvp = mvp;
  memcpy(cbvGPUAddess[renderer.frameIndex] + constantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
}

#pragma endregion
/* ------------------- RENDERING ------------------- */
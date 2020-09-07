ID3D12Debug* debugController;

/* ------------------- RENDERING ------------------- */
#pragma region WIN32
typedef struct win32_OffscreenBuffer
{
  BITMAPINFO info;
  void* memory;
  i32 width;
  i32 height;
  i32 pitch;
  i32 bytesPerPixel;
} win32_OffscreenBuffer;
global win32_OffscreenBuffer global_Backbuffer;

typedef struct win32_WindowDimension
{
  i32 width;
  i32 height;
} win32_WindowDimension;
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
  XMFLOAT4X4 mvp;
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
ID3D12Resource* renderTargets[def_FrameCount];
ID3D12CommandAllocator* commandAllocator[def_FrameCount];
ID3D12CommandQueue* commandQueue;
ID3D12RootSignature* rootSignature;
ID3D12DescriptorHeap* rtvHeap;
ID3D12PipelineState* pipelineStateObject;
ID3D12GraphicsCommandList* commandList;
UINT rtvDescSize;

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

XMFLOAT4X4 projmat;
XMFLOAT4X4 viewmat;

XMFLOAT4 cameraPos;
XMFLOAT4 cameraTarget;
XMFLOAT4 cameraUp;

XMFLOAT4X4 cube1worldmat;
XMFLOAT4X4 cube1rotmat;
XMFLOAT4 cube1pos;

XMFLOAT4X4 cube2worldmat;
XMFLOAT4X4 cube2rotmat;
XMFLOAT4 cube2offset;

i32 numCubeIndices;

ID3D12Resource* textureBuffer;
ID3D12DescriptorHeap* mainDescriptorHeap;
ID3D12Resource* textureBufferUploadHeap;

// synchronization objects
UINT frameIndex;
HANDLE fenceEvent;
ID3D12Fence* fence[def_FrameCount];
u64 fenceValue[def_FrameCount];

#pragma endregion
/* ------------------- RENDERING ------------------- */

/* ------------------- RENDERING ------------------- */
#pragma region Windows
internal win32_WindowDimension win32_GetWindowDimension(HWND Window)
{
  RECT clientRect;
  GetClientRect(Window, &clientRect);

  win32_WindowDimension windowDimension;
  windowDimension.width = clientRect.right - clientRect.left;
  windowDimension.height = clientRect.bottom - clientRect.top;
  return windowDimension;
}

internal void win32_ResizeDIBSection(win32_OffscreenBuffer* buffer, i32 width, i32 height)
{
  if (buffer->memory)
  {
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  buffer->width = width;
  buffer->height = height;
  buffer->bytesPerPixel = 4;

  buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
  buffer->info.bmiHeader.biWidth = buffer->width;
  buffer->info.bmiHeader.biHeight = -buffer->height;
  buffer->info.bmiHeader.biPlanes = 1;
  buffer->info.bmiHeader.biBitCount = 32;
  buffer->info.bmiHeader.biCompression = BI_RGB;

  i32 BitmapMemorySize = buffer->bytesPerPixel * buffer->width * buffer->height;
  buffer->memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  buffer->pitch = buffer->width * buffer->bytesPerPixel;
  // Probably should clear to black;
}

void win32_UpdateWindow(win32_OffscreenBuffer* buffer, HDC DeviceContext, i32 windowWidth, i32 windowHeight)
{
  StretchDIBits(
    DeviceContext, 0, 0, windowWidth, windowHeight,
    0, 0, buffer->width, buffer->height, buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY
  );
}
#pragma endregion

#pragma region D3D12

void InitD3D(HWND window)
{
  /* Create the device */
  IDXGIFactory4* dxgiFactory;
  hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
  win32_CheckSucceeded();

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
  win32_CheckSucceeded();

  /* Create Render Target View Command Queue */
  D3D12_COMMAND_QUEUE_DESC commandQueueDesc = { 0 };
  commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  commandQueueDesc.NodeMask = 0;
  commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
  win32_CheckSucceeded();

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
  win32_CheckSucceeded();

  swapchain = (IDXGISwapChain3*)(swapchain1);
  frameIndex = swapchain->GetCurrentBackBufferIndex();

  /* Create the backbuffers (rtvs) Descriptor Heap */
  // Create Descriptor Heap
  {
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = { 0 };
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descHeapDesc.NumDescriptors = frameCount;
    hr = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&rtvHeap));
    win32_CheckSucceeded();
    rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  // Create frame resources
  {
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (i32 i = 0; i < frameCount; i++)
    {
      // Create an rtv for each frame
      hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
      win32_CheckSucceeded();

      device->CreateRenderTargetView(renderTargets[i], NULL, cpuDescriptorHandle);

      cpuDescriptorHandle.ptr = (SIZE_T)(((INT64)cpuDescriptorHandle.ptr) + ((INT64)1) * ((INT64)rtvDescSize));
    }
  }

  for (i32 i = 0; i < frameCount; i++)
  {
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
    win32_CheckSucceeded();
  }

  // Create the command list
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], NULL, IID_PPV_ARGS(&commandList));
  win32_CheckSucceeded();

  // Create Fences & Fence Events
  for (i32 i = 0; i < frameCount; i++)
  {
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
    win32_CheckSucceeded();
    fenceValue[i] = 0;
  }

  // Create an event handle to use for frame synchronization
  fenceEvent = CreateEvent(0, false, false, 0);
  if (fenceEvent == 0)
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
    win32_CheckSucceeded();
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
  win32_CheckSucceeded();

  hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
  win32_CheckSucceeded();

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
  psoDesc.pRootSignature = rootSignature;
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
  hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
  win32_CheckSucceeded();

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
  win32_CheckSucceeded();

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
  win32_CheckSucceeded();
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
  win32_CheckSucceeded();
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
  win32_CheckSucceeded();
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
  win32_CheckSucceeded();

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
  win32_CheckSucceeded();
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
    win32_CheckSucceeded();
    constantBufferUploadHeaps[i]->SetName(L"ConstantBufferUploadHeap");

    ZeroMemory(&cbPerObject, sizeof(cbPerObject));

    CD3DX12_RANGE readRange(0, 0);
    hr = constantBufferUploadHeaps[i]->Map(0, &readRange, (void**)(&cbvGPUAddess[i]));
    win32_CheckSucceeded();

    // Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
    // so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.

    // cube 1
    memcpy(cbvGPUAddess[i], &cbPerObject, sizeof(cbPerObject));
    // cube 2
    memcpy(cbvGPUAddess[i] + constantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
  }

  // now we can create a descriptor heap that will store our srv
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = { 0 };
  heapDesc.NumDescriptors = 1;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  hr = device->CreateDescriptorHeap(
    &heapDesc, IID_PPV_ARGS(&mainDescriptorHeap)
  );
  win32_CheckSucceeded();

  // Load the image from file
  D3D12_RESOURCE_DESC textureDesc = { 0 };
  i32 imageBytesPerRow = 0;
  BYTE* imageData;
  i32 imageSize = LoadImageDataFromFile(&imageData, &textureDesc, L"test_image.png", &imageBytesPerRow);
  // make sure we have loaded the image data
  if (imageSize <= 0)
  {
    win32_running = true;
    return;
  }

  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &textureDesc,
    D3D12_RESOURCE_STATE_COPY_DEST,
    0, IID_PPV_ARGS(&textureBuffer)
  );
  win32_CheckSucceeded();
  textureBuffer->SetName(L"TextureBufferResourceHeap");

  u64 textureUploadBufferSize;
  // this function gets the size of an upload buffer needs to be to upload a texture to the gpu.
  // each row must be 256 byte aligned except for the last row which can just be the size in bytes of the row.
  device->GetCopyableFootprints(&textureDesc, 0, 1, 0, 0, 0, 0, &textureUploadBufferSize);

  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    0, IID_PPV_ARGS(&textureBufferUploadHeap)
  );
  win32_CheckSucceeded();
  textureBufferUploadHeap->SetName(L"TextureBufferUploadResourceHeap");

  // Store vertex buffer in upload heap
  D3D12_SUBRESOURCE_DATA textureData = { 0 };
  textureData.pData = &imageData[0];
  textureData.RowPitch = imageBytesPerRow;
  textureData.SlicePitch = imageBytesPerRow;

  // now we copy the upload buffer contents to the default heap
  UpdateSubresources(commandList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);

  // transition the texture default heap to a pixel shader resource
  commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

  // now we create a shader resource view
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { 0 };
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = textureDesc.Format;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  device->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


  // now we execute the command list to upload the initial assests (triangle data)
  commandList->Close();
  ID3D12CommandList* ppCommandLists[] = { commandList };
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // increment the fence value now, otherwise the buffer might not be uploaded by time we start drawing
  fenceValue[frameIndex]++;
  hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
  win32_CheckSucceeded();

  // done with image data so we can free it
  free(imageData);

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
  XMMATRIX tmpmat = XMMatrixPerspectiveFovLH(45.0f * (PI / 180.0f), aspectRatio, 0.1f, 1000.0f);
  XMStoreFloat4x4(&projmat, tmpmat);

  // set starting camera state
  cameraPos = XMFLOAT4(0.0f, 2.0f, -4.0f, 0.0f);
  cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

  // view matrix
  XMVECTOR cpos = XMLoadFloat4(&cameraPos);
  XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
  XMVECTOR cUp = XMLoadFloat4(&cameraUp);
  tmpmat = XMMatrixLookAtLH(cpos, cTarg, cUp);
  XMStoreFloat4x4(&viewmat, tmpmat);

  // set cube initial positions
  // cube1
  cube1pos = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  XMVECTOR posvec = XMLoadFloat4(&cube1pos);

  tmpmat = XMMatrixTranslationFromVector(posvec);
  XMStoreFloat4x4(&cube1rotmat, XMMatrixIdentity());
  XMStoreFloat4x4(&cube1worldmat, tmpmat);

  // cube2
  cube2offset = XMFLOAT4(1.5f, 0.0f, 0.0f, 0.0f);
  posvec = XMLoadFloat4(&cube2offset) + XMLoadFloat4(&cube1pos);

  tmpmat = XMMatrixTranslationFromVector(posvec);
  XMStoreFloat4x4(&cube2rotmat, XMMatrixIdentity());
  XMStoreFloat4x4(&cube2worldmat, tmpmat);

  OutputDebugStringA("Successfully Initialized D3D\n");
}

#pragma endregion
/* ------------------- RENDERING ------------------- */
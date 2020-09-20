struct ImageData
{
  BYTE* data;
  i32 size;
  i32 bytesPerRow;
};

void InitializeTextureFromFileName(LPCWSTR filename, ID3D12Resource** textureBuffer, ID3D12Resource** textureBufferUploadHeap, ID3D12DescriptorHeap** mainDescHeap, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
  ImageData imageData = { 0 };
  D3D12_RESOURCE_DESC textureDesc = { 0 };
  imageData.size = LoadImageDataFromFile(&imageData.data, &textureDesc, filename, &imageData.bytesPerRow);
  if (imageData.size <= 0)
  {
    win32_running = false;
    OutputDebugStringA("Failed to load test image");
    return;
  }

  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &textureDesc,
    D3D12_RESOURCE_STATE_COPY_DEST,
    0, IID_PPV_ARGS(textureBuffer)
  );
  win32_CheckSucceeded(hr);
  (*textureBuffer)->SetName(L"TextureBufferResourceHeap");

  u64 textureUploadBufferSize;
  // this function gets the size of an upload buffer needs to be to upload a texture to the gpu.
  // each row must be 256 byte aligned except for the last row which can just be the size in bytes of the row.
  device->GetCopyableFootprints(&textureDesc, 0, 1, 0, 0, 0, 0, &textureUploadBufferSize);

  hr = device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    0, IID_PPV_ARGS(textureBufferUploadHeap)
  );
  win32_CheckSucceeded(hr);
  (*textureBufferUploadHeap)->SetName(L"TextureBufferUploadResourceHeap");

  // Store texture data in upload heap
  D3D12_SUBRESOURCE_DATA textureData = { 0 };
  textureData.pData = &imageData.data[0];
  textureData.RowPitch = imageData.bytesPerRow;
  textureData.SlicePitch = imageData.bytesPerRow;

  // now we copy the upload buffer contents to the default heap
  UpdateSubresources(commandList, *textureBuffer, *textureBufferUploadHeap, 0, 0, 1, &textureData);
  // transition the texture default heap to a pixel shader resource
  commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

  // TODO: mainDescriptorHeap probably shouldn't be created here?
  // now we can create a descriptor heap that will store our srv
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = { 0 };
  heapDesc.NumDescriptors = 1;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  hr = device->CreateDescriptorHeap(
    &heapDesc, IID_PPV_ARGS(mainDescHeap)
  );
  win32_CheckSucceeded(hr);

  // now we create a shader resource view
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { 0 };
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = textureDesc.Format;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  device->CreateShaderResourceView(*textureBuffer, &srvDesc, 
    (*mainDescHeap)->GetCPUDescriptorHandleForHeapStart());
  win32_CheckSucceeded(hr);
}
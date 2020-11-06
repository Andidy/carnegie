#include "win32_renderer.h"

void UploadTextureFromImage(ImageData* imageData, i32 index, ID3D12Resource** textureBuffer, ID3D12Resource** textureBufferUploadHeap, ID3D12DescriptorHeap** mainDescHeap, ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList)
{
  D3D12_RESOURCE_DESC textureDesc = { 0 };
  LoadTextureFromImage(imageData, &textureDesc);

  hr = _device->CreateCommittedResource(
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
  _device->GetCopyableFootprints(&textureDesc, 0, 1, 0, 0, 0, 0, &textureUploadBufferSize);

  hr = _device->CreateCommittedResource(
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
  textureData.pData = &imageData->data[0];
  textureData.RowPitch = imageData->bytesPerRow;
  textureData.SlicePitch = imageData->bytesPerRow;

  // now we copy the upload buffer contents to the default heap
  UpdateSubresources(commandList, *textureBuffer, *textureBufferUploadHeap, 0, 0, 1, &textureData);
  // transition the texture default heap to a pixel shader resource
  _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

  // now we create a shader resource view
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { 0 };
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = textureDesc.Format;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;


  D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = (*mainDescHeap)->GetCPUDescriptorHandleForHeapStart();
  u32 offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  cpuDescriptorHandle.ptr = (SIZE_T)(((INT64)cpuDescriptorHandle.ptr) + ((INT64)index) * (INT64)offset);

  _device->CreateShaderResourceView(*textureBuffer, &srvDesc,
    cpuDescriptorHandle);
  win32_CheckSucceeded(hr);
}

void UploadTextureArrayFromImage(ImageData* imageData, i32 index, ID3D12Resource** textureBuffer, ID3D12Resource** textureBufferUploadHeap, ID3D12DescriptorHeap** mainDescHeap, ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList)
{
  D3D12_RESOURCE_DESC textureDesc = { 0 };
  //Convert2DTilesetDataTo1DPixelData(imageData->data, imageData->width, imageData->height,
  //  32, 32, 16, 16);
  LoadTextureArrayFromImage(imageData, &textureDesc);

  hr = _device->CreateCommittedResource(
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
  _device->GetCopyableFootprints(&textureDesc, 0, 1, 0, 0, 0, 0, &textureUploadBufferSize);

  hr = _device->CreateCommittedResource(
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
  textureData.pData = &imageData->data[0];
  textureData.RowPitch = 256*4; // has to be 256 byte aligned
  textureData.SlicePitch = 256*256*4; // has to be 512 byte aligned

  // now we copy the upload buffer contents to the default heap
  UpdateSubresources(commandList, *textureBuffer, *textureBufferUploadHeap, 0, 0, 1, &textureData);
  // transition the texture default heap to a pixel shader resource
  _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

  // now we create a shader resource view
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { 0 };
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = textureDesc.Format;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
  srvDesc.Texture2DArray.MipLevels = 1;
  srvDesc.Texture2DArray.ArraySize = 256;

  D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = (*mainDescHeap)->GetCPUDescriptorHandleForHeapStart();
  u32 offset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  cpuDescriptorHandle.ptr = (SIZE_T)(((INT64)cpuDescriptorHandle.ptr) + ((INT64)index) * (INT64)offset);

  _device->CreateShaderResourceView(*textureBuffer, &srvDesc,
    cpuDescriptorHandle);
  win32_CheckSucceeded(hr);
}
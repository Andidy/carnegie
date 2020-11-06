#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include "libs/d3dx12.h"

#include "game_platform_calls.h"
#include "carnegie.h"

struct Vertex
{
  f32 x, y, z;
  f32 r, g, b, a;
  f32 u, v;
  f32 xn, yn, zn;
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

  ID3D12RootSignature* tilemapRootSig;
  ID3D12PipelineState* tilemapPSO;

  // synchronization objects
  UINT frameIndex;
  HANDLE fenceEvent;
  ID3D12Fence* fence[def_FrameCount];
  u64 fenceValue[def_FrameCount];
};
Renderer renderer;

// app resources
struct Model
{
  ID3D12Resource* vertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
  ID3D12Resource* indexBuffer;
  D3D12_INDEX_BUFFER_VIEW indexBufferView;

  i32 numIndices;
};
Model cube;
Model quad;

ID3D12Resource* depthStencilBuffer;
ID3D12DescriptorHeap* dsDescHeap;

ConstantBufferPerObject cbPerObject;
ID3D12Resource* constantBufferUploadHeaps[frameCount];
u8* cbvGPUAddress[frameCount];

// TODO: sort out grouping the descHeaps with the textures
// add some automation here?
ID3D12DescriptorHeap* mainDescriptorHeap;

struct Texture
{
  ID3D12Resource* textureBuffer;
  ID3D12Resource* textureBufferUploadHeap;
};
Texture cat_tex;
Texture dog_tex;
Texture bird_tex;

Texture map_tex;
Texture tileset_tex;
/* win32_renderer */

global f32 aspectRatio = 1200 / 900;
global u32 global_windowWidth = 1200;
global u32 global_windowHeight = 900;

struct Vertex
{
  f32 x, y, z;
  f32 r, g, b, a;
  f32 u, v;
};

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

#pragma region D3D11

#include <d3d11.h>
#include <dxgi1_6.h>

typedef struct Renderer
{
  IDXGISwapChain* swapchain;
  ID3D11Device* device;
  ID3D11DeviceContext* deviceContext;
  ID3D11RenderTargetView* renderTargetView;
} Renderer;

void InitRenderer(HINSTANCE instance)
{

}

#pragma endregion
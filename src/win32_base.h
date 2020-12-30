#pragma once

// This stuff is only up here so i can use PIX timing data
#include <windows.h>
#define USE_PIX
#include "../libs/pix/pix3.h"

// Carnegie game and engine stuff
#include "ady_types.h"
#include "game_carnegie.cpp"

// Windows specific stuff
#include <xinput.h>
#include <dsound.h>

// DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include "../libs/d3dx12.h"

// C libs
#include <math.h>
#include <stdio.h>

#define win32_Assert(x) if(!(x)) { MessageBoxA(0, #x, "Assertion failed", MB_OK); __debugbreak(); }
#define win32_Check(x) if(!(x)) { MessageBoxA(0, #x, "Check failed", MB_OK); __debugbreak(); }
void _win32_CheckSucceeded(HRESULT hresult, char* str, int line)
{
  if (!SUCCEEDED(hresult)) {
    char win32_check_succeeded_buf[128];
#pragma warning(suppress : 4996)
    sprintf(win32_check_succeeded_buf, "%s: hr failed: %d", str, line);
    MessageBoxA(0, win32_check_succeeded_buf, "CheckSucceeded failed", MB_OK);
    __debugbreak();
  }
}
#define win32_CheckSucceeded(hresult) _win32_CheckSucceeded(hresult, __FILE__, __LINE__)

// broken out platform layer funcitonality
#include "dx_renderer_image_loading.cpp"

global b32 win32_running;
global HRESULT hr;

#include "win32_input.cpp"
#include "win32_fileio.cpp"
#include "win32_sound.cpp"

#include "dx_renderer.h"
#include "dx_renderer.cpp"
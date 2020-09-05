#ifndef WIN32_CARNEGIE_H
#define WIN32_CARNEGIE_H

#define PI 3.14159265359f
#include "ady_types.h"
#include "carnegie.cpp"
// ^^^^ Carnegie game and engine stuff

#include <windows.h>
#include <xinput.h>
#include <dsound.h>

#include <initguid.h>

#include <d3d12.h>
#include <dxgi1_6.h>
// ^^^^ Windows specific stuff

#include <d3d12.h>
// ^^^^ DirectX12

#include <math.h>
#include <stdio.h>
// ^^^^ C libs


/*

  TODO: Finish Platform Layer
  - [] Finish HMH Episodes

  - [] File I/O
    - [] SaveGame Locations
    - [] Asset Loading System
  - [] User Input
    - [] Raw Input (supporting multiple keyboards)
    - [] GetKeyboardLayout (International keyboards)
  - [] Windowing Interactions
    - [] Getting handle to own executable
    - [] Clip Cursor (multimonitor support)
    - [] WM_SetCursor (control cursor visibility)
    - [] QueryCancelAutoplay
    - [] WM_ACTIVATEAPP (for when the window isn't the focussed window)
  - [] Rendering
    - [] Sleep / Time begin period
    - [] Blit speed improvements (BitBlt)
    - [] Hardware Acceleration (DirectX12)
  - [] Sound System
    - [X] Fix hitching bug
  - [] Multi-Threading
  - more may be needed...

*/


global b32 Running;
global HRESULT hresult;

/* -------------------- MACROS --------------------- */

#define win32_Assert(x) if(!(x)) { MessageBoxA(0, #x, "Assertion failed", MB_OK); __debugbreak(); }
#define win32_Check(x) if(!(x)) { MessageBoxA(0, #x, "Check failed", MB_OK); __debugbreak(); }
#define win32_CheckSucceeded(hresult) if(!SUCCEEDED(hresult)) { MessageBoxA(0, #hresult, "CheckSucceeded failed", MB_OK); __debugbreak(); }

/* -------------------- MACROS --------------------- */

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

#pragma endregion
/* ------------------- RENDERING ------------------- */

/* ------------------ DIRECT SOUND ----------------- */

global LPDIRECTSOUNDBUFFER win32_SecondarySoundBuffer;

typedef struct win32_SoundStruct
{
  i32 bytesPerSample; // = sizeof(i16) * 2;
  i32 samplesPerSecond; // = 48000;
  u32 runningSampleIndex; // = 0;
  i32 wavePeriod; // = samplesPerSecond / toneHertz;
  i32 secondaryBufferSize; // = samplesPerSecond * bytesPerSample;
  i32 latencySampleCount; // samplesPerSecond / 15;
  f32 tsin;
} win32_SoundStruct;

/* ------------------ DIRECT SOUND ----------------- */
#endif
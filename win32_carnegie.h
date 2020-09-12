#ifndef WIN32_CARNEGIE_H
#define WIN32_CARNEGIE_H

#include "ady_types.h"
#include "carnegie.c"
// ^^^^ Carnegie game and engine stuff

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
// ^^^^ Windows specific stuff

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

global b32 win32_running;

/* -------------------- MACROS --------------------- */
global HRESULT hr;

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

/* -------------------- MACROS --------------------- */

#include "win32_input.c"
#include "win32_fileio.c"
#include "win32_sound.c"
#include "win32_renderer.c"

// ^^^^ broken out funcitonality

#endif
#include "win32_carnegie.h"

/* --------------------- XINPUT -------------------- */
#pragma region XInput
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);

X_INPUT_GET_STATE(XInputGetStateStub)
{
  return ERROR_DEVICE_NOT_CONNECTED;
}
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return ERROR_DEVICE_NOT_CONNECTED;
}

/* The assigments are in win32_LoadXInput because C doesn't like the assignments */
global x_input_get_state* XInputGetState_; // = XInputGetStateStub;
global x_input_set_state* XInputSetState_; // = XInputSetStateStub;
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void win32_LoadXInput()
{
  XInputGetState_ = XInputGetStateStub;
  XInputSetState_ = XInputSetStateStub;
  
  // If we don't have xinput1.4 check for xinput1.3
  HMODULE XInputLibrary;
  XInputLibrary = LoadLibrary("xinput1_4.dll");
  if (!XInputLibrary)
  {
    XInputLibrary = LoadLibrary("xinput9_1_0.dll");
  }
  if (!XInputLibrary)
  {
    XInputLibrary = LoadLibrary("xinput1_3.dll");
  }
  if (XInputLibrary)
  {
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  }
}

internal void win32_ProcessXInputDigitalButton(game_ButtonState* oldstate, game_ButtonState *newstate, DWORD buttonBit, DWORD xinputButtonState)
{
  newstate->endedDown = ((xinputButtonState & buttonBit) == buttonBit);
  newstate->transitionCount = (oldstate->endedDown != newstate->endedDown) ? 1 : 0;
}
#pragma endregion
/* --------------------- XINPUT -------------------- */

/* ------------------ DIRECT SOUND ----------------- */
#pragma region DSound
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void win32_InitDirectSound(HWND Window, i32 samplesPerSecond, i32 bufferSize)
{
  // Load the lib
  HMODULE DirectSoundLibrary = LoadLibrary("dsound.dll");

  if (DirectSoundLibrary)
  {
    // Get a DirectSound Object
    direct_sound_create *DirectSoundCreate = (direct_sound_create *)
      GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");
    
    LPDIRECTSOUND directSound;
    if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &directSound, 0)))
    {
      WAVEFORMATEX waveFormat = { 0 };
      waveFormat.wFormatTag = WAVE_FORMAT_PCM;
      waveFormat.nChannels = 2;
      waveFormat.wBitsPerSample = 16;
      waveFormat.nSamplesPerSec = samplesPerSecond;
      waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
      waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
      waveFormat.cbSize = 0;
      
      if (SUCCEEDED(directSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
      {
        DSBUFFERDESC bufferDescription = {0};
        bufferDescription.dwSize = sizeof(bufferDescription);
        bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
        bufferDescription.dwBufferBytes = 0;

        // Create a primary buffer
        LPDIRECTSOUNDBUFFER primaryBuffer;
        if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
        {
          if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
          {
            OutputDebugStringA("Primary Buffer Created Successfully\n");
          }
          else
          {
            // error
          }
        }
        else
        {
          // error
        }
      }
      else
      {
        // error
      }

      // Create a secondary buffer
      DSBUFFERDESC bufferDescription = { 0 };
      bufferDescription.dwSize = sizeof(bufferDescription);
      bufferDescription.dwFlags = 0;
      bufferDescription.dwBufferBytes = bufferSize;
      bufferDescription.lpwfxFormat = &waveFormat;

      if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &win32_SecondarySoundBuffer, 0)))
      {
        OutputDebugStringA("Secondary Buffer Created Successfully\n");
      }
      else
      {
        // error
      }
    }
    else
    {
      // Error
    }
  }
}

internal void win32_FillSoundBuffer(win32_SoundStruct *soundstruct, DWORD byteToLock, DWORD bytesToWrite, 
  game_SoundBuffer *soundBuffer)
{
  void* region1;
  DWORD r1size;
  void* region2;
  DWORD r2size;
  if (SUCCEEDED(win32_SecondarySoundBuffer->Lock(
    byteToLock, bytesToWrite,
    &region1, &r1size, &region2, &r2size, 0
  )))
  {
    // todo assert region sizes are correct
    DWORD region1SampleCount = r1size / soundstruct->bytesPerSample;
    i16* dstSample = (i16*)region1;
    i16* srcSample = soundBuffer->samples;
    for (DWORD sample_index = 0; sample_index < region1SampleCount; sample_index++)
    {
      *dstSample++ = *srcSample++;
      *dstSample++ = *srcSample++;
      ++soundstruct->runningSampleIndex;
    }

    DWORD region2SampleCount = r2size / soundstruct->bytesPerSample;
    dstSample = (i16*)region2;
    for (DWORD sample_index = 0; sample_index < region2SampleCount; sample_index++)
    {
      *dstSample++ = *srcSample++;
      *dstSample++ = *srcSample++;
      ++soundstruct->runningSampleIndex;
    }

    win32_SecondarySoundBuffer->Unlock(region1, r1size, region2, r2size);
  }
}

internal void win32_ClearSoundBuffer(win32_SoundStruct* soundstruct)
{
  void* region1;
  DWORD r1size;
  void* region2;
  DWORD r2size;
  if (SUCCEEDED(win32_SecondarySoundBuffer->Lock(
    0, soundstruct->secondaryBufferSize,
    &region1, &r1size, &region2, &r2size, 0
  )))
  {
    // todo assert region sizes are correct
    u8* dstSample = (u8*)region1;
    for (DWORD byte_index = 0; byte_index < r1size; byte_index++)
    {
      *dstSample++ = 0;
    }

    dstSample = (u8*)region2;
    for (DWORD byte_index = 0; byte_index < r2size; byte_index++)
    {
      *dstSample++ = 0;
    }

    win32_SecondarySoundBuffer->Unlock(region1, r1size, region2, r2size);
  }
}
#pragma endregion
/* ------------------ DIRECT SOUND ----------------- */

/* -------------------- FILE I/O ------------------- */
#pragma region File I/O
internal dev_ReadFileResult dev_ReadFile(char *filename)
{
  dev_ReadFileResult result = { 0 };
  HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (file != INVALID_HANDLE_VALUE)
  {
    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(file, &fileSize))
    {
      Assert(fileSize.QuadPart <= 0xFFFFFFFF);
      result.data = VirtualAlloc(0, fileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
      if (result.data)
      {
        DWORD bytesRead;
        if (ReadFile(file, result.data, (i32)fileSize.QuadPart, &bytesRead, 0) && (bytesRead == fileSize.QuadPart))
        {
          result.size = fileSize.QuadPart;
        }
        else
        {
          dev_FreeFile(result.data);
          result.data = 0;
        }
      }
    }
    CloseHandle(file);
  }
  return result;
}

internal void dev_FreeFile(void *memory)
{
  if (memory)
  {
    VirtualFree(memory, 0, MEM_RELEASE);
  }
}

internal b32 dev_WriteFile(char *filename, u32 memorySize, void *memory)
{
  b32 result = false;
  HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
  if (file != INVALID_HANDLE_VALUE)
  {
    DWORD bytesWritten;
    if (WriteFile(file, memory, memorySize, &bytesWritten, 0))
    {
      result = bytesWritten == memorySize;
    }
    else
    {
      
    }
    CloseHandle(file);
  }
  return result;
}
#pragma endregion
/* -------------------- FILE I/O ------------------- */

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
  if(buffer->memory)
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
  buffer->memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
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

#pragma endregion
/* ------------------- RENDERING ------------------- */

/* -------------------- WINDOWS -------------------- */
LRESULT CALLBACK win32_MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
  LRESULT Result = 0;
  
  switch(Message)
  {
    case WM_SIZE:
    {
      OutputDebugStringA("WM_SIZE\n");
    } break;
    
    case WM_DESTROY:
    {
      Running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    
    case WM_CLOSE:
    {
      Running = false;
      OutputDebugStringA("WM_CLOSE\n");
    } break;
    
    case WM_ACTIVATEAPP:
    {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    
    case WM_KEYUP:
    case WM_KEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
    {
      u32 vkCode = (u32)WParam;
      b32 wasDown = ((LParam & (1 << 30)) != 0);
      b32 isDown = ((LParam & (1 << 31)) == 0);

      if (isDown != wasDown)
      {
        if (vkCode == 'W')
        {
          OutputDebugStringA("W: ");
          if (isDown)
          {
            OutputDebugStringA("is Down\n");
          }
          if (wasDown)
          {
            OutputDebugStringA("was Down\n");
          }
        }
        if (vkCode == 'A')
        {

        }
        if (vkCode == 'S')
        {

        }
        if (vkCode == 'D')
        {

        }
        if (vkCode == VK_UP)
        {

        }
        if (vkCode == VK_DOWN)
        {

        }
        if (vkCode == VK_LEFT)
        {

        }
        if (vkCode == VK_RIGHT)
        {

        }
        if (vkCode == VK_SPACE)
        {

        }
        if (vkCode == VK_ESCAPE)
        {

        }
      }

      b32 altKeyDown = ((LParam & (1 << 29)) != 0);
      if ((vkCode == VK_F4) && altKeyDown)
      {
        Running = false;
      }
    } break;

    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window, &Paint);
      //i32 x = Paint.rcPaint.left;
      //i32 y = Paint.rcPaint.top;
      //i32 width = Paint.rcPaint.right - Paint.rcPaint.left;
      //i32 height = Paint.rcPaint.bottom - Paint.rcPaint.top;
      
      win32_WindowDimension dim = win32_GetWindowDimension(Window);
      win32_UpdateWindow(&global_Backbuffer, DeviceContext, dim.width, dim.height);
      EndPaint(Window, &Paint);
    } break;
    
    default:
    {
      Result = DefWindowProc(Window, Message, WParam, LParam);
    } break;
  }
  
  return Result;
}

i32 CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
  /* XINPUT Function Pointers to handle LIB/DLL Loading */
  win32_LoadXInput();

  //global_OffscreenBuffer.info;
  global_Backbuffer.memory = 0;
  global_Backbuffer.width = 0;
  global_Backbuffer.height = 0;

  // Timing Info
  /*
  LARGE_INTEGER perftimerfreqresult;
  QueryPerformanceFrequency(&perftimerfreqresult);
  i64 perftimerfreq = perftimerfreqresult.QuadPart;
  */
  
  // Windows Window
  WNDCLASSA WindowClass = {0};
  WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
  WindowClass.lpfnWndProc = win32_MainWindowCallback;
  WindowClass.hInstance = Instance;
  // WindowClass.hIcon = ;
  WindowClass.lpszClassName = "CarnegieWindowClass";

  if(RegisterClass(&WindowClass))
  {
    HWND Window = CreateWindowExA(
      0,
      WindowClass.lpszClassName,
      "Project Carnegie",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT,
      0, 0, Instance, 0
    );

    win32_WindowDimension dim = win32_GetWindowDimension(Window);
    win32_ResizeDIBSection(&global_Backbuffer, dim.width, dim.height);

    if(Window)
    {
      HDC deviceContext = GetDC(Window);
      
      // Sound Init
      win32_SoundStruct soundstruct = { 0 };
      soundstruct.bytesPerSample = sizeof(i16) * 2;
      soundstruct.samplesPerSecond = 48000;
      soundstruct.runningSampleIndex = 0;
      soundstruct.wavePeriod = soundstruct.samplesPerSecond / 256;
      soundstruct.secondaryBufferSize = soundstruct.samplesPerSecond * soundstruct.bytesPerSample;
      soundstruct.latencySampleCount = soundstruct.samplesPerSecond / 15;
      win32_InitDirectSound(Window, soundstruct.samplesPerSecond, soundstruct.secondaryBufferSize);
      win32_ClearSoundBuffer(&soundstruct);
      win32_SecondarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

      i16 *samples = (i16 *)VirtualAlloc(0, soundstruct.secondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

      // Configs and Tickers
      Running = true;
      
      /*
      LARGE_INTEGER lasttimer;
      QueryPerformanceCounter(&lasttimer);
      u64 lastcyclecount = __rdtsc();
      */
      
      game_Input gameInput[2] = { 0 };
      game_Input *newInput = &gameInput[0];
      game_Input *oldInput = &gameInput[1];

      game_Memory gameMemory = { 0 };
      gameMemory.isInitialized = false;
      gameMemory.size = Gigabytes((u64)4);
      gameMemory.data = (u8*)VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      gameMemory.scratchsize = Gigabytes((u64)2);
      gameMemory.scratchdata = (u8*)VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

      // GAME LOOP ----------------------------------------------
      while(Running)
      {
        // Input
        MSG Message;
        while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
          if (Message.message == WM_QUIT)
          {
            Running = false;
          }

          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }

        // should we poll more frequently?
        i32 maxControllerCount = XUSER_MAX_COUNT;
        if (maxControllerCount > ArrayCount(newInput->controllers))
        {
          maxControllerCount = ArrayCount(newInput->controllers);
        }
        for (i32 controllerIndex = 0; controllerIndex < maxControllerCount; controllerIndex++)
        {
          game_ControllerState* oldcontroller = &oldInput->controllers[controllerIndex];
          game_ControllerState* newcontroller = &newInput->controllers[controllerIndex];

          XINPUT_STATE controllerState;
          if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
          {
            // Controller is connected
            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
            
            // todo handle dpad
            // b32 dpad_up    = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            // b32 dpad_down  = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            // b32 dpad_left  = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            // b32 dpad_right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            
            newcontroller->isAnalog = true;
            newcontroller->lstartx = oldcontroller->lendx;
            newcontroller->lstarty = oldcontroller->lendy;
            newcontroller->rstartx = oldcontroller->rendx;
            newcontroller->rstarty = oldcontroller->rendy;

            f32 lx, ly, rx, ry;
            if (pad->sThumbLX < 0)
            {
              lx = (f32)pad->sThumbLX / (f32)32768;
            }
            else
            {
              lx = (f32)pad->sThumbLX / (f32)32767;
            }

            if (pad->sThumbLY < 0)
            {
              ly = (f32)pad->sThumbLY / (f32)32768;
            }
            else
            {
              ly = (f32)pad->sThumbLY / (f32)32767;
            }
            
            if (pad->sThumbRX < 0)
            {
              rx = (f32)pad->sThumbRX / (f32)32768;
            }
            else
            {
              rx = (f32)pad->sThumbRX / (f32)32767;
            }
            
            if (pad->sThumbRY < 0)
            {
              ry = (f32)pad->sThumbRY / (f32)32768;
            }
            else
            {
              ry = (f32)pad->sThumbRY / (f32)32767;
            }

            newcontroller->lminx = newcontroller->lmaxx = newcontroller->lendx = lx;
            newcontroller->lminy = newcontroller->lmaxy = newcontroller->lendy = ly;
            newcontroller->rminx = newcontroller->rmaxx = newcontroller->rendx = rx;
            newcontroller->rminy = newcontroller->rmaxy = newcontroller->rendy = ry;

            win32_ProcessXInputDigitalButton(
              &oldcontroller->a, &newcontroller->a, XINPUT_GAMEPAD_A, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->b, &newcontroller->b, XINPUT_GAMEPAD_B, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->x, &newcontroller->x, XINPUT_GAMEPAD_X, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->y, &newcontroller->y, XINPUT_GAMEPAD_Y, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->start, &newcontroller->start, XINPUT_GAMEPAD_START, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->select, &newcontroller->select, XINPUT_GAMEPAD_BACK, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->l1, &newcontroller->l1, XINPUT_GAMEPAD_LEFT_SHOULDER, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->r1, &newcontroller->r1, XINPUT_GAMEPAD_RIGHT_SHOULDER, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->l3, &newcontroller->l3, XINPUT_GAMEPAD_LEFT_THUMB, pad->wButtons
            );
            win32_ProcessXInputDigitalButton(
              &oldcontroller->r3, &newcontroller->r3, XINPUT_GAMEPAD_RIGHT_THUMB, pad->wButtons
            );
          }
          else
          {
            // Controller is not connected / avalible
          }
        }

        // DirectSound Output Test
        DWORD playCursor = 0;
        DWORD writeCursor = 0;
        DWORD byteToLock = 0;
        DWORD targetCursor = 0;
        DWORD bytesToWrite = 0;
        b32 soundIsValid = false;
        if (SUCCEEDED(win32_SecondarySoundBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
        {
          byteToLock = (soundstruct.runningSampleIndex * soundstruct.bytesPerSample) % soundstruct.secondaryBufferSize;

          targetCursor = ((playCursor + (soundstruct.latencySampleCount * soundstruct.bytesPerSample)) % soundstruct.secondaryBufferSize);

          if (byteToLock > targetCursor)
          {
            bytesToWrite = (soundstruct.secondaryBufferSize - byteToLock);
            bytesToWrite += targetCursor;
          }
          else
          {
            bytesToWrite = targetCursor - byteToLock;
          }

          soundIsValid = true;
        }

        // Rendering
        game_OffscreenBuffer buffer = { 0 };
        buffer.memory = global_Backbuffer.memory;
        buffer.width = global_Backbuffer.width;
        buffer.height = global_Backbuffer.height;
        buffer.pitch = global_Backbuffer.pitch;

        //i16 samples[48000 * 2];
        game_SoundBuffer soundBuffer = { 0 };
        soundBuffer.samplesPerSecond = soundstruct.samplesPerSecond;
        soundBuffer.sampleCount = bytesToWrite / soundstruct.bytesPerSample;
        soundBuffer.samples = samples;

        GameUpdateAndRender(&gameMemory, newInput, &buffer, &soundBuffer);

        // Direct Sound Test Continued
        if(soundIsValid)
        {
          win32_FillSoundBuffer(&soundstruct, byteToLock, bytesToWrite, &soundBuffer);
        }

        // Drawing
        dim = win32_GetWindowDimension(Window);
        win32_UpdateWindow(&global_Backbuffer, deviceContext, dim.width, dim.height);
        
        #pragma region timing
        /*
        LARGE_INTEGER endtimer;
        QueryPerformanceCounter(&endtimer);
        i64 timerelapsed = endtimer.QuadPart - lasttimer.QuadPart;
        f32 msperframe = (f32)((1000.0f * (f32)timerelapsed) / (f32)perftimerfreq);
        i64 fps = (i64)(perftimerfreq / timerelapsed);

        u64 endcyclecount = __rdtsc();
        u64 cycleselapsed = endcyclecount - lastcyclecount;

        char str_buffer[256];
        sprintf(str_buffer, "ms / frame: %f, fps: %I64d, %I64u\n", msperframe, fps, cycleselapsed);
        OutputDebugStringA(str_buffer);

        lasttimer = endtimer;
        lastcyclecount = endcyclecount;
        */
        #pragma endregion

        game_Input* temp = newInput;
        newInput = oldInput;
        oldInput = temp;
      }
    }
  }
  
  return 0;
}
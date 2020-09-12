#include "win32_carnegie.h"

u32 counter = 0;

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
      win32_running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    
    case WM_CLOSE:
    {
      win32_running = false;
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
        win32_running = false;
      }
    } break;

    /*case WM_PAINT:
    {
      
    } break;*/

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
    HWND window = CreateWindowExA(
      0,
      WindowClass.lpszClassName,
      "Project Carnegie",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT,
      global_windowWidth, global_windowHeight,
      0, 0, Instance, 0
    );

    if(window)
    {
      //HDC deviceContext = GetDC(window);
      
      // Sound Init
      win32_SoundStruct soundstruct = { 0 };
      soundstruct.bytesPerSample = sizeof(i16) * 2;
      soundstruct.samplesPerSecond = 48000;
      soundstruct.runningSampleIndex = 0;
      soundstruct.wavePeriod = soundstruct.samplesPerSecond / 256;
      soundstruct.secondaryBufferSize = soundstruct.samplesPerSecond * soundstruct.bytesPerSample;
      soundstruct.latencySampleCount = soundstruct.samplesPerSecond / 15;
      win32_InitDirectSound(window, soundstruct.samplesPerSecond, soundstruct.secondaryBufferSize);
      win32_ClearSoundBuffer(&soundstruct);
      win32_SecondarySoundBuffer->lpVtbl->Play(
        win32_SecondarySoundBuffer, 0, 0, DSBPLAY_LOOPING
      );

      i16* samples = (i16*)VirtualAlloc(0, soundstruct.secondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

      // Configs and Tickers
      win32_running = true;

      /*
      LARGE_INTEGER lasttimer;
      QueryPerformanceCounter(&lasttimer);
      u64 lastcyclecount = __rdtsc();
      */

      game_Input gameInput[2] = { 0 };
      game_Input* newInput = &gameInput[0];
      game_Input* oldInput = &gameInput[1];

      game_Memory gameMemory = { 0 };
      gameMemory.isInitialized = false;
      gameMemory.size = Gigabytes((u64)4);
      gameMemory.data = (u8*)VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      gameMemory.scratchsize = Gigabytes((u64)2);
      gameMemory.scratchdata = (u8*)VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

      // GAME LOOP ----------------------------------------------

      while(win32_running)
      {
        ++counter;
        
        // Input
        MSG Message;
        while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
          if (Message.message == WM_QUIT)
          {
            win32_running = false;
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
        if (SUCCEEDED(win32_SecondarySoundBuffer->lpVtbl->GetCurrentPosition(
          win32_SecondarySoundBuffer, &playCursor, &writeCursor)))
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

        //i16 samples[48000 * 2];
        game_SoundBuffer soundBuffer = { 0 };
        soundBuffer.samplesPerSecond = soundstruct.samplesPerSecond;
        soundBuffer.sampleCount = bytesToWrite / soundstruct.bytesPerSample;
        soundBuffer.samples = samples;

        GameUpdateAndRender(&gameMemory, newInput, &soundBuffer);

        // Direct Sound Test Continued
        if(soundIsValid)
        {
          win32_FillSoundBuffer(&soundstruct, byteToLock, bytesToWrite, &soundBuffer);
        }

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
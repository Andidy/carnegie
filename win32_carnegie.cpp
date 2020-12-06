#include "win32_carnegie.h"

u32 counter = 0;

LRESULT CALLBACK win32_MainWindowCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  LRESULT result = 0;
  
  switch(message)
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
      win32_Assert("Keyboard event in windows callback function");
    } break;

    /*case WM_PAINT:
    {
      
    } break;*/

    default:
    {
      result = DefWindowProc(window, message, wparam, lparam);
    } break;
  }
  
  return result;
}

i32 CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
  hr = SetThreadDescription(GetCurrentThread(), L"simulation_thread");
  if (FAILED(hr))
  {
    // Call failed.
  }
  
  /* XINPUT Function Pointers to handle LIB/DLL Loading */
  win32_LoadXInput();

  // Timing Info
  
  LARGE_INTEGER perftimerfreqresult;
  QueryPerformanceFrequency(&perftimerfreqresult);
  i64 perftimerfreq = perftimerfreqresult.QuadPart;
  

  window_width = 1200;
  window_height = 900;

  // Windows Window
  WNDCLASSA windowClass = {0};
  windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
  windowClass.lpfnWndProc = win32_MainWindowCallback;
  windowClass.hInstance = instance;
  // WindowClass.hIcon = ;
  windowClass.lpszClassName = "CarnegieWindowClass";

  if(RegisterClass(&windowClass))
  {
    HWND window = CreateWindowExA(
      0,
      windowClass.lpszClassName,
      "Project Carnegie",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT,
      window_width, window_height,
      0, 0, instance, 0
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
      win32_SecondarySoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

      i16* samples = (i16*)VirtualAlloc(0, soundstruct.secondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

      // Configs and Tickers
      win32_running = true;

      
      LARGE_INTEGER lasttimer;
      QueryPerformanceCounter(&lasttimer);
      u64 lastcyclecount = __rdtsc();
      

      game_Input gameInput[2] = { 0 };
      game_Input* newInput = &gameInput[0];
      game_Input* oldInput = &gameInput[1];

      game_Memory gameMemory = { 0 };
      gameMemory.isInitialized = false;
      gameMemory.size = Gigabytes((u64)6);
      gameMemory.data = (u8*)VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      gameMemory.scratchsize = Gigabytes((u64)2);
      gameMemory.scratchdata = (u8*)VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      
      // Probably want to change this to being its own function call
      GameUpdateAndPrepareRenderData(0, &gameMemory, newInput, NULL);

      /* Enable D3D12 Debug layers */
      /*
      hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
      win32_CheckSucceeded(hr);

      debugController->EnableDebugLayer();
      */
      // Init D3D12
      InitD3D(window, &gameMemory);


      // GAME LOOP ----------------------------------------------

      while(win32_running)
      {
        #pragma region timing

        LARGE_INTEGER endtimer;
        QueryPerformanceCounter(&endtimer);
        i64 timerelapsed = endtimer.QuadPart - lasttimer.QuadPart;
        f32 msperframe = (f32)((1000.0f * (f32)timerelapsed) / (f32)perftimerfreq);
        i64 fps = (i64)(perftimerfreq / timerelapsed);

        u64 endcyclecount = __rdtsc();
        u64 cycleselapsed = endcyclecount - lastcyclecount;

        char str_buffer[256];
        sprintf_s(str_buffer, "ms / frame: %f, fps: %I64d, %I64u\n", msperframe, fps, cycleselapsed);
        OutputDebugStringA(str_buffer);

        lasttimer = endtimer;
        lastcyclecount = endcyclecount;

        INT pix_color = PIX_COLOR(255, 0, 0);
        PIXBeginEvent(pix_color, "WIN32_LOOP");

        #pragma endregion
        
        ++counter;
        
        // Input
        *newInput = { 0 };
        for (i32 i = 0; i < NUM_KEYBOARD_BUTTONS; i++)
        {
          newInput->keyboard.buttons[i].endedDown = oldInput->keyboard.buttons[i].endedDown;
        }
        win32_UpdateInput(newInput);

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

        //i16 samples[48000 * 2];
        game_SoundBuffer soundBuffer = { 0 };
        soundBuffer.samplesPerSecond = soundstruct.samplesPerSecond;
        soundBuffer.sampleCount = bytesToWrite / soundstruct.bytesPerSample;
        soundBuffer.samples = samples;

        win32_WindowDimension dim = win32_GetWindowDimension(window);
        window_width = dim.width;
        window_height = dim.height;
        window_aspectRatio = (f32)window_width / (f32)window_height;

        GameUpdateAndPrepareRenderData(msperframe, &gameMemory, newInput, &soundBuffer);

        // Direct Sound Test Continued
        if(soundIsValid)
        {
          win32_FillSoundBuffer(&soundstruct, byteToLock, bytesToWrite, &soundBuffer);
        }

        Render(window, &gameMemory);

        game_Input* temp = newInput;
        newInput = oldInput;
        oldInput = temp;

        PIXEndEvent();
      }
    }
  }
  
  return 0;
}
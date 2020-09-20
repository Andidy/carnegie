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
    XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
  }
}

internal void win32_ProcessXInputDigitalButton(game_ButtonState* oldstate, game_ButtonState* newstate, DWORD buttonBit, DWORD xinputButtonState)
{
  newstate->endedDown = ((xinputButtonState & buttonBit) == buttonBit);
  newstate->transitionCount = (oldstate->endedDown != newstate->endedDown) ? 1 : 0;
}

internal void win32_ProcessKeyboardMessage(game_ButtonState *newState, b32 isDown)
{
  win32_Assert(newState->endedDown != isDown);
  newState->endedDown = isDown;
  ++newState->transitionCount;
}

internal void win32_UpdateInput()
{
  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
  {
    switch(message.message)
    {
      case WM_QUIT:
      {
        win32_running = false;
      } break;

      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP:
      {
        u32 vkCode = (u32)message.wParam;
        b32 wasDown = ((message.lParam & (1 << 30)) != 0);
        b32 isDown = ((message.lParam & (1 << 31)) == 0);

        if (isDown != wasDown)
        {
          switch (vkCode)
          {
            case 'W':
            {

            } break;
            case 'A':
            {

            } break;
            case 'S':
            {

            } break;
            case 'D':
            {

            } break;
            case VK_UP:
            {

            } break;
            case VK_DOWN:
            {

            } break;
            case VK_LEFT:
            {

            } break;
            case VK_RIGHT:
            {

            } break;
            case VK_SPACE:
            {

            } break;
            case VK_ESCAPE:
            {

            } break;
          }
        }

        b32 altKeyDown = ((message.lParam & (1 << 29)) != 0);
        if ((vkCode == VK_F4) && altKeyDown)
        {
          win32_running = false;
        }
      } break;

      default:
      {
        TranslateMessage(&message);
        DispatchMessage(&message);
      } break;
    }
  }
}

#pragma endregion
/* --------------------- XINPUT -------------------- */
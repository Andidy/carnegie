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
#pragma endregion
/* --------------------- XINPUT -------------------- */
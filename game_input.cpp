#include "game_input.h"

b32 keyPressed(game_ButtonState button)
{
  return (button.endedDown) && (button.transitionCount > 0);
}

b32 keyReleased(game_ButtonState button)
{
  return !(button.endedDown) && (button.transitionCount > 0);
}

b32 keyDown(game_ButtonState button)
{
  return (button.endedDown) && (button.transitionCount == 0);
}
#pragma once

#include "ady_types.h"

namespace Game
{
  struct ButtonState
  {
    i32 transitionCount;
    b32 endedDown;
  };

  struct ControllerState
  {
    b32 isAnalog;
    f32 lstartx;
    f32 lstarty;
    f32 lmaxx;
    f32 lmaxy;
    f32 lminx;
    f32 lminy;
    f32 lendx;
    f32 lendy;

    f32 rstartx;
    f32 rstarty;
    f32 rmaxx;
    f32 rmaxy;
    f32 rminx;
    f32 rminy;
    f32 rendx;
    f32 rendy;

    union
    {
      ButtonState buttons[16];
      struct
      {
        ButtonState a; // cross
        ButtonState b; // circle
        ButtonState x; // square
        ButtonState y; // triangle
        ButtonState up;
        ButtonState down;
        ButtonState left;
        ButtonState right;
        ButtonState l1;
        ButtonState r1;
        ButtonState l2;
        ButtonState r2;
        ButtonState l3;
        ButtonState r3;
        ButtonState start;
        ButtonState select;
      };
    };
  };

  const int NUM_KEYBOARD_BUTTONS = 32;
  struct KeyboardState
  {
    union
    {
      struct
      {
        ButtonState a;
        ButtonState b;
        ButtonState c;
        ButtonState d;
        ButtonState e;
        ButtonState f;
        ButtonState g;
        ButtonState h;
        ButtonState i;
        ButtonState j;
        ButtonState k;
        ButtonState l;
        ButtonState m;
        ButtonState n;
        ButtonState o;
        ButtonState p;
        ButtonState q;
        ButtonState r;
        ButtonState s;
        ButtonState t;
        ButtonState u;
        ButtonState v;
        ButtonState w;
        ButtonState x;
        ButtonState y;
        ButtonState z;

        ButtonState up;
        ButtonState down;
        ButtonState left;
        ButtonState right;

        ButtonState space;
        ButtonState escape;
      };

      ButtonState buttons[NUM_KEYBOARD_BUTTONS];
    };
  };

  struct Input
  {
    KeyboardState keyboard;
  };

  /////////////// Functions /////////////////////
  
  b32 keyPressed(ButtonState button)
  {
    return (button.endedDown) && (button.transitionCount > 0);
  }

  b32 keyReleased(ButtonState button)
  {
    return !(button.endedDown) && (button.transitionCount > 0);
  }

  b32 keyDown(ButtonState button)
  {
    return (button.endedDown) && (button.transitionCount == 0);
  }
}
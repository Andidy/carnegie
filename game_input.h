#pragma once

#include "ady_types.h"

struct game_ButtonState
{
  i32 transitionCount;
  b32 endedDown;
};

struct game_ControllerState
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
    game_ButtonState buttons[16];
    struct
    {
      game_ButtonState a; // cross
      game_ButtonState b; // circle
      game_ButtonState x; // square
      game_ButtonState y; // triangle
      game_ButtonState up;
      game_ButtonState down;
      game_ButtonState left;
      game_ButtonState right;
      game_ButtonState l1;
      game_ButtonState r1;
      game_ButtonState l2;
      game_ButtonState r2;
      game_ButtonState l3;
      game_ButtonState r3;
      game_ButtonState start;
      game_ButtonState select;
    };
  };
};

const int NUM_KEYBOARD_BUTTONS = 32;
struct game_KeyboardState
{
  union
  {
    struct
    {
      game_ButtonState a;
      game_ButtonState b;
      game_ButtonState c;
      game_ButtonState d;
      game_ButtonState e;
      game_ButtonState f;
      game_ButtonState g;
      game_ButtonState h;
      game_ButtonState i;
      game_ButtonState j;
      game_ButtonState k;
      game_ButtonState l;
      game_ButtonState m;
      game_ButtonState n;
      game_ButtonState o;
      game_ButtonState p;
      game_ButtonState q;
      game_ButtonState r;
      game_ButtonState s;
      game_ButtonState t;
      game_ButtonState u;
      game_ButtonState v;
      game_ButtonState w;
      game_ButtonState x;
      game_ButtonState y;
      game_ButtonState z;

      game_ButtonState up;
      game_ButtonState down;
      game_ButtonState left;
      game_ButtonState right;

      game_ButtonState space;
      game_ButtonState escape;
    };

    game_ButtonState buttons[NUM_KEYBOARD_BUTTONS];
  };
};

struct game_Input
{
  game_KeyboardState keyboard;
};
#ifndef CARNEGIE_H
#define CARNEGIE_H

#include "ady_types.h"
#include "ady_math.h"
#include <math.h>

#include "game_entity.h"

// Macros //
#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])

#define Kilobytes(val) (1024 * (val))
#define Megabytes(val) (1024 * Kilobytes(val))
#define Gigabytes(val) (1024 * Megabytes(val))

#define Assert(val) if(!(val)) {*(int *)0 = 0;}

// PLATFORM LAYER -> GAME //
/*
  These variables and functions are platform specific but their results are
  needed / used by the Game layer, and so are defined here, but are 
  implemented inside the platform layer. For example, the Game is saved and so
  needs to write the save file to disk.
*/
struct dev_ReadFileResult
{
  u64 size;
  void* data;
};

internal dev_ReadFileResult dev_ReadFile(char *filename);
internal void dev_FreeFile(void *memory);
internal b32 dev_WriteFile(char *filename, u32 memorySize, void *memory);

// GAME -> PLATFORM LAYER //
/*
  These are the Game data types and functions which the platform layer may
  need to access for various reasons. For example, the game_KeyboardState
  needs to have its key states set by the platform layer.
*/
// Input, Render Buffer, Timing, Sound Buffer

u32 window_width;
u32 window_height;
f32 window_aspectRatio;

struct game_SoundBuffer
{
  i32 samplesPerSecond;
  i32 sampleCount;
  i16* samples;
};

struct game_ButtonState
{
  i32 transitionCount;
  b32 endedDown;
};

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

struct game_Memory
{
  b32 isInitialized;
  u64 size;
  void* data; // NOTE: Must be cleared to ZERO at startup
  u64 scratchsize;
  void* scratchdata; // NOTE: Must be cleared to ZERO at startup
};

internal void GameUpdateAndPrepareRenderData(game_Memory* game_Memory, game_Input* Input, game_SoundBuffer* soundBuffer);

/* Game Only */

const i32 NUM_ENTITIES = 3;
struct game_State
{
  i32 xoff;
  i32 yoff;
  i32 toneHertz;

  Camera camera;
  Entity entities[NUM_ENTITIES];
};

#endif
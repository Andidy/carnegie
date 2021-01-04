#pragma once

#include "ady_types.h"
#include "ady_math.h"

// ========================================================
// Macros
#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])

#define Kilobytes(val) (1024 * (val))
#define Megabytes(val) (1024 * Kilobytes(val))
#define Gigabytes(val) (1024 * Megabytes(val))

#define Assert(val) if(!(val)) {*(int *)0 = 0;}

// end Macros
// ========================================================
// Memory

struct Memory {
  b32 isInitialized;
  u64 size;
  void* data; // NOTE: Must be cleared to ZERO at startup
  u64 scratchsize;
  void* scratchdata; // NOTE: Must be cleared to ZERO at startup
};

// end Memory
// ========================================================
// Input

struct ButtonState {
  i32 transitionCount;
  b32 endedDown;
};

struct ControllerState {
  b32 isAnalog;
  f32 lstartx, lstarty;
  f32 lmaxx, lmaxy;
  f32 lminx, lminy;
  f32 lendx, lendy;

  f32 rstartx, rstarty;
  f32 rmaxx, rmaxy;
  f32 rminx, rminy;
  f32 rendx, rendy;

  union {
    ButtonState buttons[16];
    struct {
      ButtonState a, b, x, y;
      ButtonState up, down, left, right;
      ButtonState l1, r1;
      ButtonState l2, r2;
      ButtonState l3, r3;
      ButtonState start, select;
    };
  };
};

const int NUM_KEYBOARD_BUTTONS = 32;
struct KeyboardState {
  union {
    ButtonState buttons[NUM_KEYBOARD_BUTTONS];
    struct {
      ButtonState a, b, c, d, e, f, g, h, i, j, k, l,
        m, n, o, p, q, r, s, t, u, v, w, x, y, z;
      ButtonState up, down, left, right;
      ButtonState space, escape;
    };
  };
};

struct Input {
  KeyboardState keyboard;
};

// ------------ Functions -----------------

b32 keyPressed(ButtonState button) {
  return (button.endedDown) && (button.transitionCount > 0);
}

b32 keyReleased(ButtonState button) {
  return !(button.endedDown) && (button.transitionCount > 0);
}

b32 keyDown(ButtonState button) {
  return (button.endedDown) && (button.transitionCount == 0);
}

// end Input
// ========================================================
// Graphics

u32 window_width;
u32 window_height;
f32 window_aspectRatio;

struct Image {
  uchar* data;
  u32 width;
  u32 height;
  i32 size;
  i32 bytesPerRow;
  // i32 format_enum_num; ??? to store dxgi format from LoadImageFromDisk
  //                          to use in LoadTextureFromImage's format assignment
};

// end Graphics
// ========================================================
// I/O

struct dev_ReadFileResult {
  u64 size;
  void* data;
};

internal dev_ReadFileResult dev_ReadFile(char* filename);
internal void dev_FreeFile(void* memory);
internal b32 dev_WriteFile(char* filename, u32 memorySize, void* memory);

internal i32 LoadImageFromDisk(char* filename, Image* image);

// end I/O
// ========================================================
// Audio

struct SoundBuffer {
  i32 samplesPerSecond;
  i32 sampleCount;
  i16* samples;
};

// end Audio
// ========================================================
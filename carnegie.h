#ifndef CARNEGIE_H
#define CARNEGIE_H

#include "ady_types.h"
#include "ady_math.h"
#include <math.h>

// Macros //
#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])

#define Kilobytes(val) (1024 * (val))
#define Megabytes(val) (1024 * Kilobytes(val))
#define Gigabytes(val) (1024 * Megabytes(val))

#define Assert(val) if(!(val)) {*(int *)0 = 0;}

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

struct game_Memory
{
  b32 isInitialized;
  u64 size;
  void* data; // NOTE: Must be cleared to ZERO at startup
  u64 scratchsize;
  void* scratchdata; // NOTE: Must be cleared to ZERO at startup
};

#include "game_platform_calls.h"
#include "game_input.h"

#include "game_entity.h"

#include "game_proc_gen.h"
#include "game_tileset_resolver.h"

internal void GameUpdateAndPrepareRenderData(f32 dt, game_Memory* game_Memory, Game::Input* Input, game_SoundBuffer* soundBuffer);

/* Game Only */

const i32 NUM_ENTITIES = 6;
const i32 NUM_UNITS = 5;
const i32 UNIT_ANIM_OFFSET = 7;
struct game_State
{
  f32 dt;

  i32 toneHertz;

  Camera camera;
  Entity entities[NUM_ENTITIES];

  ImageData cat_img;
  ImageData dog_img;
  ImageData bird_img;

  ImageData map_img;
  ImageData map2_img;
  ImageData tileset_img;
  ImageData tileset2_img;

  f32 anim_timer;
  i32 anim_counter;
  b32 anim_reset;

  b32 moved_unit;

  Unit unit[NUM_UNITS];

  ImageData blank_unit_img;
  ImageData unit_img;
  ImageData unit_horseman_img;
  ImageData unit_archer_img;
};

#endif
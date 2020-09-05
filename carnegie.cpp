#include "carnegie.h"

/* --------------- RENDERING --------------- */

internal void RenderWeirdGradient(game_OffscreenBuffer* buffer, i32 xoff, i32 yoff)
{
  i32 width = buffer->width;
  i32 height = buffer->height;
  i32 pitch = buffer->pitch;

  u8* row = (u8*)buffer->memory;
  for (i32 y = 0; y < height; ++y)
  {
    u32* pixel = (u32*)row;
    for (i32 x = 0; x < width; ++x)
    {
      u8 blue = (u8)(x + xoff);
      u8 green = (u8)(y + yoff);
      *pixel++ = ((green << 8) | blue);
    }
    row += pitch;
  }
}

/* --------------- RENDERING --------------- */

/* --------------- SOUND --------------- */
#define PI 3.14159265359f
internal void GameOutputSound(game_SoundBuffer *soundBuffer, i32 toneHertz)
{
  local f32 tsin;
  i16 toneVolume = 1000;
  i32 wavePeriod = soundBuffer->samplesPerSecond / toneHertz;

  i16 *sampleOut = soundBuffer->samples;
  for (i32 sample_index = 0; sample_index < soundBuffer->sampleCount; sample_index++)
  {
    f32 sinvalue = sinf(tsin);
    i16 sampleValue = (i16)(sinvalue * toneVolume);
    *sampleOut++ = sampleValue;
    *sampleOut++ = sampleValue;

    tsin += 2.0f * PI * (1.0f / (f32)wavePeriod);
  }
}

/* --------------- SOUND --------------- */

internal void GameUpdateAndRender(game_Memory *gameMemory, game_Input *Input, game_OffscreenBuffer *offscreenBuffer, game_SoundBuffer *soundBuffer)
{
  game_State* gameState = (game_State *)gameMemory->data;
  if (!gameMemory->isInitialized)
  {
    //gameState->xoff = 0;
    //gameState->yoff = 0;
    gameState->toneHertz = 256;
    gameMemory->isInitialized = true;

    char* c = __FILE__;
    dev_ReadFileResult d= dev_ReadFile(c);
    if (d.data)
    {
      dev_FreeFile(d.data);
      char* test_arr = "This is my test string. Weeeeeee!\n Second line\n Third line!";
      dev_WriteFile("test.txt", 60, test_arr);
    }
  }

  game_ControllerState playerOne = Input->controllers[0];
  if (playerOne.isAnalog)
  {
    // Use Analog Tuning
    gameState->toneHertz = 256 + (i32)(128.0f * (playerOne.lendy));
    gameState->xoff += (i32)(4.0f * playerOne.rendy);
  }
  else
  {
    // Use Digital Tuning
  }

  if (playerOne.a.endedDown)
  {
    gameState->yoff++;
  }
  
  // todo: allow sample offsets here for more robust platform options
  GameOutputSound(soundBuffer, gameState->toneHertz);
  
  RenderWeirdGradient(offscreenBuffer, gameState->xoff, gameState->yoff);
}
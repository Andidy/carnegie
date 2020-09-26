#include "carnegie.h"

/* --------------- SOUND --------------- */
#define PI 3.14159265359f
internal void GameOutputSound(game_SoundBuffer *soundBuffer, i32 toneHertz)
{
  local f32 tsin;
  i16 toneVolume = 100;
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

internal void GameUpdateAndPrepareRenderData(game_Memory* gameMemory, game_Input* Input, game_SoundBuffer* soundBuffer)
{
  game_State* gameState = (game_State*)gameMemory->data;
  if (!gameMemory->isInitialized)
  {
    gameState->toneHertz = 256;
    gameMemory->isInitialized = true;
    gameState->entities[0] = { {0, 0, 0}, {0, 0, 0} };
    gameState->entities[1] = { {1, 0, 0}, {-0.0001f, 0, 0} };
  }

  gameState->entities[1].pos.x += gameState->entities[1].vel.x;
  if ((gameState->entities[1].pos.x < -1) || (gameState->entities[1].pos.x > 1))
  {
    gameState->entities[1].vel.x *= -1;
  }

  // todo: allow sample offsets here for more robust platform options
  GameOutputSound(soundBuffer, gameState->toneHertz);
}
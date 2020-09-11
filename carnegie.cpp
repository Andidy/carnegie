#include "carnegie.h"

/* --------------- RENDERING --------------- */

/* --------------- RENDERING --------------- */

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

internal void GameUpdateAndRender(game_Memory *gameMemory, game_Input *Input, game_SoundBuffer *soundBuffer)
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
}
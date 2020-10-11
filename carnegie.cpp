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
    gameMemory->isInitialized = true;
    gameState->toneHertz = 256;
    
    // build proj and view matrix
    mat4 projmat = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);

    // set starting camera state
    vec3 cameraPos = Vec3(0.0f, 2.0f, -4.0f);
    vec3 cameraTarget = Vec3(0.0f, 0.0f, 0.0f);
    vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f);

    // view matrix
    mat4 viewmat = LookAtMat(cameraPos, cameraTarget, cameraUp);

    gameState->camera = {cameraPos, cameraTarget, cameraUp, projmat, viewmat};
    
    gameState->entities[0] = { {0, 0, 0}, {0, 0.0001f, 0}, {1, 1, 1} };
    gameState->entities[1] = { {1, 0, 0}, {-0.0001f, 0, 0}, {0.5, 0.5, 0.5} };
    gameState->entities[2] = { {1, -0.125f, 0}, {0, 0, -0.0003f}, {0.25, 0.25, 0.25} };
  }

  if (keyDown(Input->keyboard.a))
  {
    gameState->entities[0].pos.y += gameState->entities[0].vel.y;
    if ((gameState->entities[0].pos.y < -1) || (gameState->entities[0].pos.y > 1))
    {
      gameState->entities[0].vel.y *= -1;
    }
  }

  gameState->entities[1].pos.x += gameState->entities[1].vel.x;
  if ((gameState->entities[1].pos.x < -1) || (gameState->entities[1].pos.x > 1))
  {
    gameState->entities[1].vel.x *= -1;
  }

  gameState->entities[2].pos.z += gameState->entities[2].vel.z;
  if ((gameState->entities[2].pos.z < -1) || (gameState->entities[2].pos.z > 1))
  {
    gameState->entities[2].vel.z *= -1;
  }

  gameState->camera.proj = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);
  gameState->camera.view = LookAtMat(gameState->camera.pos, gameState->camera.target, gameState->camera.up);

  // todo: allow sample offsets here for more robust platform options
  GameOutputSound(soundBuffer, gameState->toneHertz);
}
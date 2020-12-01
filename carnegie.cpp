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








void UpdateCamera(Camera* camera, game_Input* input, game_State* gameState)
{
  f32 speed = 0.001f * gameState->dt;

  if (keyDown(input->keyboard.space))
  {
    speed *= 100;
  }

  if (keyDown(input->keyboard.w))
  {
    camera->pos.y += speed;
    camera->target.y += speed;
  }

  if (keyDown(input->keyboard.s))
  {
    camera->pos.y -= speed;
    camera->target.y -= speed;
  }

  if (keyDown(input->keyboard.a))
  {
    camera->pos.x -= speed;
    camera->target.x -= speed;
  }

  if (keyDown(input->keyboard.d))
  {
    camera->pos.x += speed;
    camera->target.x += speed;
  }

  if (keyDown(input->keyboard.r))
  {
    camera->pos.z += speed;
    camera->target.z += speed;
  }

  if (keyDown(input->keyboard.f))
  {
    camera->pos.z -= speed;
    camera->target.z -= speed;
  }

  camera->view = LookAtMat(camera->pos, camera->target, camera->up);
}

internal void GameUpdateAndPrepareRenderData(f32 dt, game_Memory* gameMemory, game_Input* input, game_SoundBuffer* soundBuffer)
{
  game_State* gameState = (game_State*)gameMemory->data;
  
  // Probably want to change this to being its own function call
  if (!gameMemory->isInitialized)
  {
    gameMemory->isInitialized = true;
    gameState->dt = dt;
    gameState->anim_timer = 0.0f;
    gameState->toneHertz = 256;
    
    // build proj and view matrix
    mat4 projmat = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);

    // set starting camera state
    vec3 cameraPos = Vec3(107.0f, 670.0f, -26.0f);
    vec3 cameraTarget = Vec3(107.0f, 670.0f, 0.0f);
    vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f);

    // view matrix
    mat4 viewmat = LookAtMat(cameraPos, cameraTarget, cameraUp);

    gameState->camera = {cameraPos, cameraTarget, cameraUp, projmat, viewmat};
    
    gameState->entities[0] = { {0, 0, 0}, {0, 0.0001f, 0}, {1, 1, 1}, -1};
    gameState->entities[1] = { {1, 0, 0}, {-0.0001f, 0, 0}, {0.5, 0.5, 0.5}, -1 };
    gameState->entities[2] = { {1, -0.125f, 0}, {0, 0, -0.0003f}, {0.25, 0.25, 0.25}, -1 };
    gameState->entities[3] = { {0, 0, 1.002f }, {0, 0, 0}, {4320, 2160, 1}, 0, 3, 5 };
    gameState->entities[4] = { {0, 0, 1.001f }, {0, 0, 0}, {4320, 2160, 1}, 0, 4, 5 };

    gameState->entities[5] = { {0, 0, 1.000f }, {0, 0, 0}, {4320, 2160, 1}, 1, 6, 7 };

    gameState->unit[0] = { {2269, 405}, 0, 1 };
    gameState->unit[1] = { {2271, 406}, 1, 3 };
    gameState->unit[2] = { {2267, 405}, 1, 4 };
    gameState->unit[3] = { {2269, 409}, 1, 0 };
    gameState->unit[4] = { {2265, 401}, 0, 2 };

    LoadImageFromDisk("../test_assets/cat.png", &(gameState->cat_img));
    LoadImageFromDisk("../test_assets/dog.png", &(gameState->dog_img));
    LoadImageFromDisk("../test_assets/bird.png", &(gameState->bird_img));

    LoadImageFromDisk("../test_assets/map_data.png", &(gameState->map_img));
    LoadImageFromDisk("../test_assets/map2_data.png", &(gameState->map2_img));
    LoadImageFromDisk("../test_assets/tileset.png", &(gameState->tileset_img));

    LoadImageFromDisk("../test_assets/unit_data.png", &(gameState->unit_img));
    LoadImageFromDisk("../test_assets/horseman.png", &(gameState->unit_horseman_img));
    LoadImageFromDisk("../test_assets/archer.png", &(gameState->unit_archer_img));

    ProcGen();

    return;
  }

  gameState->dt = dt;
  gameState->anim_timer += dt;
  if (gameState->anim_timer > 250.0f)
  {
    gameState->anim_counter = (gameState->anim_counter + 1) % 4;
    gameState->anim_timer = 0.0f;
  }

  UpdateCamera(&gameState->camera, input, gameState);

  if (keyReleased(input->keyboard.a))
  {
    vec2 pos = gameState->unit[0].pos;
    pos.x -= 1.0f;
    gameState->unit[0].pos = pos;
  }

  /*
  gameState->entities[0].pos.y += gameState->entities[0].vel.y;
  if ((gameState->entities[0].pos.y < -1) || (gameState->entities[0].pos.y > 1))
  {
    gameState->entities[0].vel.y *= -1;
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
  }*/

  for (i32 i = 0; i < NUM_UNITS; i++)
  {
    gameState->unit_img.data[(int)(4 * gameState->unit[i].pos.x + gameState->unit[i].pos.y * gameState->unit_img.bytesPerRow + 1)] = (uchar)gameState->unit[i].animation;
    gameState->unit_img.data[(int)(4 * gameState->unit[i].pos.x + gameState->unit[i].pos.y * gameState->unit_img.bytesPerRow + 2)] = (uchar)(gameState->unit[i].type);
  }

  gameState->camera.proj = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);
  gameState->camera.view = LookAtMat(gameState->camera.pos, gameState->camera.target, gameState->camera.up);

  // todo: allow sample offsets here for more robust platform options
  GameOutputSound(soundBuffer, gameState->toneHertz);
}
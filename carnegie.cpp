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

void MarchUnit(Unit* unit, Direction dir, vec2 new_pos)
{
  unit->dir = dir;
  unit->new_pos = new_pos;
}

void MoveUnit(Unit* unit)
{
  unit->old_pos = unit->pos;
  unit->pos = unit->new_pos;

  if (unit->dir == WEST)
  {
    unit->animation = 2;
  }
  else if (unit->dir == EAST)
  {
    unit->animation = 3;
  }
  else if (unit->dir == SOUTH)
  {
    unit->animation = 4;
  }
  else if (unit->dir == NORTH)
  {
    unit->animation = 5;
  }
}

void CleanupUnit(Unit* unit)
{
  unit->old_pos = unit->pos;
  unit->new_pos = unit->pos;
  unit->animation = 0;
  unit->dir = NONE;
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
    gameState->moved_unit = 0;
    gameState->anim_reset = 0;

    // build proj and view matrix
    mat4 projmat = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);

    // set starting camera state
    vec3 cameraPos = Vec3(107.0f, 670.0f, -26.0f);
    vec3 cameraTarget = Vec3(107.0f, 670.0f, 0.0f);
    vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f);

    // view matrix
    mat4 viewmat = LookAtMat(cameraPos, cameraTarget, cameraUp);

    gameState->camera = { cameraPos, cameraTarget, cameraUp, projmat, viewmat };

    gameState->entities[0] = { {0, 0, 0}, {0, 0.0001f, 0}, {1, 1, 1}, -1 };
    gameState->entities[1] = { {1, 0, 0}, {-0.0001f, 0, 0}, {0.5, 0.5, 0.5}, -1 };
    gameState->entities[2] = { {1, -0.125f, 0}, {0, 0, -0.0003f}, {0.25, 0.25, 0.25}, -1 };
    gameState->entities[3] = { {0, 0, 1.002f }, {0, 0, 0}, {4320, 2160, 1}, 0, 3, 5 };
    gameState->entities[4] = { {0, 0, 1.001f }, {0, 0, 0}, {4320, 2160, 1}, 0, 4, 5 };

    gameState->entities[5] = { {0, 0, 1.000f }, {0, 0, 0}, {4320, 2160, 1}, 1, 6, 7 };

    gameState->unit[0] = { {2269, 405}, {2269, 405}, {2269, 405}, NONE, 0, 0 };
    gameState->unit[1] = { {2271, 406}, {2271, 406}, {2271, 406}, NONE, 0, 0 };
    gameState->unit[2] = { {2267, 405}, {2267, 405}, {2267, 405}, NONE, 1, 0 };
    gameState->unit[3] = { {2269, 409}, {2269, 409}, {2269, 409}, NONE, 0, 0 };
    gameState->unit[4] = { {2265, 401}, {2265, 401}, {2265, 401}, NONE, 0, 0 };

    LoadImageFromDisk("../test_assets/cat.png", &(gameState->cat_img));
    LoadImageFromDisk("../test_assets/dog.png", &(gameState->dog_img));
    LoadImageFromDisk("../test_assets/bird.png", &(gameState->bird_img));

    LoadImageFromDisk("../test_assets/map_data.png", &(gameState->map_img));
    LoadImageFromDisk("../test_assets/map2_data.png", &(gameState->map2_img));
    LoadImageFromDisk("../test_assets/tileset.png", &(gameState->tileset_img));

    LoadImageFromDisk("../test_assets/unit_data.png", &(gameState->blank_unit_img));
    ImageData* src = &(gameState->blank_unit_img);
    ImageData* dest = &(gameState->unit_img);

    dest->bytesPerRow = src->bytesPerRow;
    dest->height = src->height;
    dest->width = src->width;
    dest->size = src->size;
    dest->data = (uchar*)malloc(dest->size);
    memcpy(dest->data, src->data, src->size);
    
    LoadImageFromDisk("../test_assets/horseman.png", &(gameState->unit_horseman_img));
    LoadImageFromDisk("../test_assets/archer.png", &(gameState->unit_archer_img));

    ProcGen();

    return;
  }

  gameState->anim_reset = 0;

  gameState->dt = dt;
  gameState->anim_timer += dt;
  if (gameState->anim_timer > 125.0f)
  {
    gameState->anim_counter = (gameState->anim_counter + 1) % 4;
    gameState->anim_timer = 0.0f;
    if (gameState->anim_counter == 0)
    {
      gameState->anim_reset = 1;
    }
  }

  INT color = PIX_COLOR(0, 0, 255);
  PIXBeginEvent(color, "USER INPUT");
  UpdateCamera(&gameState->camera, input, gameState);

  if (keyReleased(input->keyboard.left))
  {
    vec2 pos = gameState->unit[0].pos;
    pos.x -= 1.0f;
    MarchUnit(&(gameState->unit[0]), WEST, pos);
    gameState->moved_unit = 1;
  }
  if (keyReleased(input->keyboard.right))
  {
    vec2 pos = gameState->unit[0].pos;
    pos.x += 1.0f;
    MarchUnit(&(gameState->unit[0]), EAST, pos);
    gameState->moved_unit = 1;
  }
  if (keyReleased(input->keyboard.down))
  {
    vec2 pos = gameState->unit[0].pos;
    pos.y += 1.0f;
    MarchUnit(&(gameState->unit[0]), SOUTH, pos);
    gameState->moved_unit = 1;
  }
  if (keyReleased(input->keyboard.up))
  {
    vec2 pos = gameState->unit[0].pos;
    pos.y -= 1.0f;
    MarchUnit(&(gameState->unit[0]), NORTH, pos);
    gameState->moved_unit = 1;
  }
  
  if (gameState->anim_reset)
  {
    //gameState->moved_unit = 0;
    color = PIX_COLOR(0, 255, 255);
    PIXBeginEvent(color, "CLEAR UNIT DATA");
    
    ImageData* src = &(gameState->blank_unit_img);
    ImageData* dest = &(gameState->unit_img);

    dest->bytesPerRow = src->bytesPerRow;
    dest->height = src->height;
    dest->width = src->width;
    dest->size = src->size;
    memcpy(dest->data, src->data, src->size);

    PIXEndEvent();
  }

  PIXEndEvent();
  
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
    if (gameState->anim_reset)
    {
      if (gameState->unit[i].animation != 0)
      {
        CleanupUnit(&(gameState->unit[i]));
      }
      else if (gameState->unit[i].dir != NONE)
      {
        MoveUnit(&(gameState->unit[i]));
      }
    }
    
    vec2 pos = gameState->unit[i].pos;
    vec2 old_pos = gameState->unit[i].old_pos;

    gameState->unit_img.data[(int)(4 * pos.x + pos.y * gameState->unit_img.bytesPerRow + 1)] = (uchar)gameState->unit[i].animation;
    gameState->unit_img.data[(int)(4 * pos.x + pos.y * gameState->unit_img.bytesPerRow + 2)] = (uchar)(gameState->unit[i].type);

    i32 anim = gameState->unit[i].animation;
    if (anim == 0 || anim == 1) // stationary
    {
      gameState->unit_img.data[(int)(4 * pos.x + pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)0;
      gameState->unit_img.data[(int)(4 * old_pos.x + old_pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)0;
    }
    if (anim == 2) // left
    {
      gameState->unit_img.data[(int)(4 * pos.x + pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)7;
      gameState->unit_img.data[(int)(4 * old_pos.x + old_pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)8;
    }
    if (anim == 3) // right
    {
      gameState->unit_img.data[(int)(4 * pos.x + pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)4;
      gameState->unit_img.data[(int)(4 * old_pos.x + old_pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)3;
    }
    if (anim == 4) // down
    {
      gameState->unit_img.data[(int)(4 * pos.x + pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)5;
      gameState->unit_img.data[(int)(4 * old_pos.x + old_pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)6;
    }
    if (anim == 5) // up
    {
      gameState->unit_img.data[(int)(4 * pos.x + pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)2;
      gameState->unit_img.data[(int)(4 * old_pos.x + old_pos.y * gameState->unit_img.bytesPerRow + 0)] = (uchar)1;
    }

    gameState->unit_img.data[(int)(4 * old_pos.x + old_pos.y * gameState->unit_img.bytesPerRow + 1)] = (uchar)gameState->unit[i].animation;
    gameState->unit_img.data[(int)(4 * old_pos.x + old_pos.y * gameState->unit_img.bytesPerRow + 2)] = (uchar)(gameState->unit[i].type);
  }

  gameState->camera.proj = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);
  gameState->camera.view = LookAtMat(gameState->camera.pos, gameState->camera.target, gameState->camera.up);

  // todo: allow sample offsets here for more robust platform options
  GameOutputSound(soundBuffer, gameState->toneHertz);
}
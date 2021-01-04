#pragma once

#include "universal.cpp"

#include "game_entity.h"
#include "game_tileset.h"

internal void GameUpdateAndPrepareRenderData(f32 dt, Memory * game_Memory, Input * Input, SoundBuffer * soundBuffer);

/* Game Only */

const i32 NUM_ENTITIES = 7;
const i32 NUM_UNITS = 5;
const i32 UNIT_ANIM_OFFSET = 7;
struct GameState {
  f32 dt;

  i32 toneHertz;

  Camera camera;
  Entity entities[NUM_ENTITIES];

  Image cat_img;
  Image dog_img;
  Image bird_img;

  Tilemap tilemap;

  Image map_img;
  Image map2_img;
  Image tileset_img;
  Image tileset2_img;

  f32 anim_timer;
  i32 anim_counter;
  b32 anim_reset;

  b32 moved_unit;

  Unit unit[NUM_UNITS];

  Image blank_unit_img;
  Image unit_img;
  Image unit_horseman_img;
  Image unit_archer_img;

  i32 prev_camera_index;
  Image ui_img;
  Image ui_data_img;
};

// Data
// ============================================================================
// Functions

// ============================================================================
// Sound
internal void GameOutputSound(SoundBuffer* soundBuffer, i32 toneHertz) {
  local f32 tsin;
  i16 toneVolume = 100;
  i32 wavePeriod = soundBuffer->samplesPerSecond / toneHertz;

  i16* sampleOut = soundBuffer->samples;
  for (i32 sample_index = 0; sample_index < soundBuffer->sampleCount; sample_index++) {
    f32 sinvalue = sinf(tsin);
    i16 sampleValue = (i16)(sinvalue * toneVolume);
    *sampleOut++ = sampleValue;
    *sampleOut++ = sampleValue;

    tsin += 2.0f * PI * (1.0f / (f32)wavePeriod);
  }
}

// ============================================================================
// Game
void UpdateCamera(Camera* camera, Input* input, GameState* gameState) {
  f32 speed = 0.01f * gameState->dt;

  if (keyDown(input->keyboard.space)) {
    speed *= 100;
  }

  if (keyDown(input->keyboard.w)) {
    camera->pos.y += speed;
    camera->target.y += speed;
  }

  if (keyDown(input->keyboard.s)) {
    camera->pos.y -= speed;
    camera->target.y -= speed;
  }

  if (keyDown(input->keyboard.a)) {
    camera->pos.x -= speed;
    camera->target.x -= speed;
  }

  if (keyDown(input->keyboard.d)) {
    camera->pos.x += speed;
    camera->target.x += speed;
  }

  if (keyDown(input->keyboard.r)) {
    camera->pos.z += sqrtf(0.02f * speed * (f32)fabs((double)camera->pos.z));
    if (camera->pos.z > 0.0f) {
      camera->pos.z = -0.01f;
    }
  }

  if (keyDown(input->keyboard.f)) {
    camera->pos.z -= sqrtf(0.02f * speed * (f32)fabs((double)camera->pos.z));
    if (camera->pos.z < -800.0f) {
      camera->pos.z = -800.0f;
    }
  }

  camera->view = LookAtMat(camera->pos, camera->target, camera->up);
}

void MarchUnit(Unit* unit, Direction dir, vec2 new_pos) {
  unit->dir = dir;
  unit->new_pos = new_pos;
}

void MoveUnit(Unit* unit) {
  unit->old_pos = unit->pos;
  unit->pos = unit->new_pos;

  if (unit->dir == WEST) {
    unit->animation = 2;
  }
  else if (unit->dir == EAST) {
    unit->animation = 3;
  }
  else if (unit->dir == SOUTH) {
    unit->animation = 4;
  }
  else if (unit->dir == NORTH) {
    unit->animation = 5;
  }
}

void CleanupUnit(Unit* unit) {
  unit->old_pos = unit->pos;
  unit->new_pos = unit->pos;
  unit->animation = 0;
  unit->dir = NONE;
}

internal void GameUpdateAndPrepareRenderData(f32 dt, Memory* gameMemory, Input* input, SoundBuffer* soundBuffer) {
  GameState* gameState = (GameState*)gameMemory->data;

  // Probably want to change this to being its own function call
  if (!gameMemory->isInitialized) {
    gameMemory->isInitialized = true;
    gameState->dt = dt;
    gameState->anim_timer = 0.0f;
    gameState->toneHertz = 256;
    gameState->moved_unit = 0;
    gameState->anim_reset = 0;
    
    // build proj and view matrix
    mat4 projmat = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);

    // set starting camera state
    vec3 cameraPos = Vec3(2269.0f, 2160.0f - 405.0f, -26.0f);
    vec3 cameraTarget = Vec3(2269.0f, 2160.0f - 405.0f, 0.0f);
    vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f);

    // view matrix
    mat4 viewmat = LookAtMat(cameraPos, cameraTarget, cameraUp);

    gameState->camera = { cameraPos, cameraTarget, cameraUp, projmat, viewmat };

    gameState->entities[0] = { {0, 0, 0}, {0, 0.0001f, 0}, {1, 1, 1}, -1 };
    gameState->entities[1] = { {1, 0, 0}, {-0.0001f, 0, 0}, {0.5, 0.5, 0.5}, -1 };
    gameState->entities[2] = { {1, -0.125f, 0}, {0, 0, -0.0003f}, {0.25, 0.25, 0.25}, -1 };

    gameState->entities[3] = { {2160, 1080, 1.15f}, {0, 0, 0}, {4320, 2160, 1}, 0, 3, 5 };
    gameState->entities[4] = { {2160, 1080, 1.1f}, {0, 0, 0}, {4320, 2160, 1}, 0, 4, 9 };

    gameState->entities[5] = { {2160, 1080, 1.05f}, {0, 0, 0}, {4320, 2160, 1}, 1, 6, 7 };
    
    gameState->prev_camera_index = 0;
    gameState->entities[6] = { {2160, 1080, 1.0f}, {0, 0, 0}, {4320, 2160, 1}, 0, 11, 10 };

    gameState->unit[0] = { {2269, 405}, {2269, 405}, {2269, 405}, NONE, 0, 0 };
    gameState->unit[1] = { {2271, 406}, {2271, 406}, {2271, 406}, NONE, 0, 0 };
    gameState->unit[2] = { {2267, 405}, {2267, 405}, {2267, 405}, NONE, 1, 0 };
    gameState->unit[3] = { {2269, 409}, {2269, 409}, {2269, 409}, NONE, 0, 0 };
    gameState->unit[4] = { {2265, 401}, {2265, 401}, {2265, 401}, NONE, 0, 0 };

    gameState->tilemap = { 4320, 2160, 0 };
    gameState->tilemap.tiles = (Tile*)calloc(4320 * 2160, sizeof(Tile)); // replace with an allocator at some point

    LoadImageFromDisk("../test_assets/cat.png", &(gameState->cat_img));
    LoadImageFromDisk("../test_assets/dog.png", &(gameState->dog_img));
    LoadImageFromDisk("../test_assets/bird.png", &(gameState->bird_img));

    LoadImageFromDisk("../test_assets/map_data.png", &(gameState->map_img));
    LoadImageFromDisk("../test_assets/map2_data.png", &(gameState->map2_img));
    LoadImageFromDisk("../test_assets/tileset.png", &(gameState->tileset_img));
    LoadImageFromDisk("../test_assets/tiled_tileset_test.png", &(gameState->tileset2_img));

    LoadImageFromDisk("../test_assets/unit_data.png", &(gameState->blank_unit_img));

    LoadImageFromDisk("../test_assets/ui.png", &(gameState->ui_img));
    LoadImageFromDisk("../test_assets/ui_data.png", &(gameState->ui_data_img));

    Image* src = &(gameState->blank_unit_img);
    Image* dest = &(gameState->unit_img);

    dest->bytesPerRow = src->bytesPerRow;
    dest->height = src->height;
    dest->width = src->width;
    dest->size = src->size;
    dest->data = (uchar*)malloc(dest->size);
    memcpy(dest->data, src->data, src->size);

    LoadImageFromDisk("../test_assets/horseman.png", &(gameState->unit_horseman_img));
    LoadImageFromDisk("../test_assets/archer.png", &(gameState->unit_archer_img));

    ResolveTilingForTileset(&(gameState->map2_img));

    return;
  }

  gameState->anim_reset = 0;

  gameState->dt = dt;
  gameState->anim_timer += dt;
  if (gameState->anim_timer > 125.0f) {
    gameState->anim_counter = (gameState->anim_counter + 1) % 4;
    gameState->anim_timer = 0.0f;
    if (gameState->anim_counter == 0) {
      gameState->anim_reset = 1;
    }
  }

  // ======================================================
  // Process user input

  INT color = PIX_COLOR(0, 0, 255);
  PIXBeginEvent(color, "USER INPUT");

  UpdateCamera(&gameState->camera, input, gameState);

  if (keyReleased(input->keyboard.left)) {
    vec2 pos = gameState->unit[0].pos;
    pos.x -= 1.0f;
    MarchUnit(&(gameState->unit[0]), WEST, pos);
    gameState->moved_unit = 1;
  }
  if (keyReleased(input->keyboard.right)) {
    vec2 pos = gameState->unit[0].pos;
    pos.x += 1.0f;
    MarchUnit(&(gameState->unit[0]), EAST, pos);
    gameState->moved_unit = 1;
  }
  if (keyReleased(input->keyboard.down)) {
    vec2 pos = gameState->unit[0].pos;
    pos.y += 1.0f;
    MarchUnit(&(gameState->unit[0]), SOUTH, pos);
    gameState->moved_unit = 1;
  }
  if (keyReleased(input->keyboard.up)) {
    vec2 pos = gameState->unit[0].pos;
    pos.y -= 1.0f;
    MarchUnit(&(gameState->unit[0]), NORTH, pos);
    gameState->moved_unit = 1;
  }

  PIXEndEvent();
  
  // end Process User Input
  // ======================================================
  
  // reset the unit animation image
  if (gameState->anim_reset) {
    //gameState->moved_unit = 0;
    color = PIX_COLOR(0, 255, 255);
    PIXBeginEvent(color, "CLEAR UNIT DATA");

    Image* src = &(gameState->blank_unit_img);
    Image* dest = &(gameState->unit_img);

    dest->bytesPerRow = src->bytesPerRow;
    dest->height = src->height;
    dest->width = src->width;
    dest->size = src->size;
    memcpy(dest->data, src->data, src->size);

    PIXEndEvent();
  }

  // Update Units
  for (i32 i = 0; i < NUM_UNITS; i++) {
    Unit* unit = &(gameState->unit[i]);
    
    if (gameState->anim_reset) {
      if (unit->animation != 0) {
        CleanupUnit(unit);
      }
      else if (unit->dir != NONE) {
        MoveUnit(unit);
      }
    }

    ivec2 pos = IVec2((i32)unit->pos.x, (i32)unit->pos.y);
    ivec2 old_pos = IVec2((i32)unit->old_pos.x, (i32)unit->old_pos.y);

    i32 bytesPerRow = gameState->unit_img.bytesPerRow;

    gameState->unit_img.data[(int)Index(pos.x, pos.y, 1, bytesPerRow)] = (uchar)unit->animation;
    gameState->unit_img.data[(int)Index(pos.x, pos.y, 2, bytesPerRow)] = (uchar)unit->type;

    i32 anim = unit->animation;
    if (anim == 0 || anim == 1) { // stationary
      gameState->unit_img.data[(int)Index(pos.x, pos.y, 0, bytesPerRow)] = (uchar)0;
      gameState->unit_img.data[(int)Index(old_pos.x, old_pos.y, 0, bytesPerRow)] = (uchar)0;
    }
    else if (anim == 2) { // left
      gameState->unit_img.data[(int)Index(pos.x, pos.y, 0, bytesPerRow)] = (uchar)7;
      gameState->unit_img.data[(int)Index(old_pos.x, old_pos.y, 0, bytesPerRow)] = (uchar)8;
    }
    else if (anim == 3) { // right
      gameState->unit_img.data[(int)Index(pos.x, pos.y, 0, bytesPerRow)] = (uchar)4;
      gameState->unit_img.data[(int)Index(old_pos.x, old_pos.y, 0, bytesPerRow)] = (uchar)3;
    }
    else if (anim == 4) { // down
      gameState->unit_img.data[(int)Index(pos.x, pos.y, 0, bytesPerRow)] = (uchar)5;
      gameState->unit_img.data[(int)Index(old_pos.x, old_pos.y, 0, bytesPerRow)] = (uchar)6;
    }
    else if (anim == 5) { // up
      gameState->unit_img.data[(int)Index(pos.x, pos.y, 0, bytesPerRow)] = (uchar)2;
      gameState->unit_img.data[(int)Index(old_pos.x, old_pos.y, 0, bytesPerRow)] = (uchar)1;
    }

    gameState->unit_img.data[(int)Index(old_pos.x, old_pos.y, 1, bytesPerRow)] = (uchar)unit->animation;
    gameState->unit_img.data[(int)Index(old_pos.x, old_pos.y, 2, bytesPerRow)] = (uchar)unit->type;
  }

  int index = (int)Index((i32)gameState->camera.pos.x, 2160 - (i32)(gameState->camera.pos.y + 0.5f), 1, gameState->ui_data_img.bytesPerRow);
  gameState->ui_data_img.data[index] = 1;
  if (index != gameState->prev_camera_index)     {
    gameState->ui_data_img.data[gameState->prev_camera_index] = 0;
    gameState->prev_camera_index = index;
  }
  
  gameState->camera.proj = PerspectiveMat(45.0f, window_aspectRatio, 0.1f, 1000.0f);
  gameState->camera.view = LookAtMat(gameState->camera.pos, gameState->camera.target, gameState->camera.up);

  // todo: allow sample offsets here for more robust platform options
  GameOutputSound(soundBuffer, gameState->toneHertz);
}
#pragma once

struct Entity
{
  vec3 pos;
  vec3 vel;
  vec3 scale;

  i32 is_animated;
  i32 data_layer;
  i32 spritesheet_base;
};

struct Unit
{
  vec2 pos;
  i32 type;
  i32 animation;
};
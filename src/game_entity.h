#pragma once

struct Camera {
  vec3 pos;
  vec3 target;
  vec3 up;

  mat4 proj;
  mat4 view;
};

struct Entity {
  vec3 pos;
  vec3 vel;
  vec3 scale;

  i32 is_animated;
  i32 data_layer;
  i32 tileset_base_index;
};

enum Direction {
  NONE,
  WEST,
  EAST,
  SOUTH,
  NORTH,
  NUM_DIRECTIONS
};

struct Tilemap {
  // ???
};

struct Unit {
  vec2 old_pos;
  vec2 pos;
  vec2 new_pos;

  Direction dir;

  i32 type;
  i32 animation;
};
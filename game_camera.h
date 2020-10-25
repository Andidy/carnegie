#pragma once

#include "ady_types.h"

struct Camera
{
  vec3 pos;
  vec3 target;
  vec3 up;

  mat4 proj;
  mat4 view;
};
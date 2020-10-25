#include "game_proc_gen.h"

#include "ady_types.h"

#include <math.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

void ProcGen()
{
  u32 seed = 123;
  u32 map_width = 2000;
  u32 map_height = 1000;

  u8* heightmap = (u8*)malloc(sizeof(u8) * map_width * map_height);

  // Generate a random offset based on seed
  srand(seed);
  i32 xoffset = rand() % 2048;
  i32 yoffset = rand() % 2048;

  // loop through every location
  for (u32 y = 0; y < map_height; y++)
  {
    for (u32 x = 0; x < map_width; x++)
    {
      // generate inital noise layer
      f32 frequency = 2.0;
      u32 octaves = 3;
      f32 amplitude = 1.0;
      f32 range = 1.0;

      f32 posx = ((x / (f32)map_width) - 0.5f) * frequency;
      f32 posy = ((y / (f32)map_height) - 0.5f) * frequency;

      f32 n = noise(posx + xoffset, posy + yoffset);

      // layer more noise onto the inital noise to create a more organic image
      for (u32 o = 0; o < octaves; o++)
      {
        frequency = frequency * 2.0f;
        amplitude = amplitude * 0.5f;
        range = range + amplitude;
        n = n + noise(posx * frequency, posy * frequency)
          * amplitude;
      }
      n = n / range;

      if (n < 0)
      {
        n = 0;
      }
      else if (n > 1)
      {
        n = 1;
      }
      n = powf(n, 2.0f);
      n *= 255.0f;

      // assign the generated noise data to its tile
      heightmap[y * map_width + x] = (u8)n;
    }
  }

  stbi_write_bmp("heightmap.bmp", map_width, map_height, 1, (void*)heightmap);
}
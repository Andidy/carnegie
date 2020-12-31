#pragma once

i32 Index(i32 x, i32 y, i32 bytesPerRow) {
  if (x < 0) x = 4319;
  if (y < 0) y = 2159;
  if (x >= 4320) x = 0;
  if (y >= 2160) y = 0;
  return y * bytesPerRow + x * 4 + 1;
}

void ResolveTileset(Image* img) {
  // i32 size = img->size;
  u32 width = 4320; // img->width;
  u32 height = 2160; // img->height;
  i32 bpr = img->bytesPerRow;
  u8 result = 0;

  u8* result_data = (u8*)calloc(4320 * 2160, 4);

  for (i32 y = 0; y < (i32)height; y++) {
    for (i32 x = 0; x < (i32)width; x++) {
      result = 0;
      if (img->data[Index(x, y, bpr)] == 0) {
        result = 1;
      }
      else {
        if ((img->data[Index(x - 1, y - 1, bpr)] > 0) &&
          ((img->data[Index(x - 1, y + 0, bpr)]) || (img->data[Index(x + 0, y - 1, bpr)]))) {
          result += 1;
        }
        if (img->data[Index(x + 0, y - 1, bpr)] > 0) {
          result += 2;
        }
        if ((img->data[Index(x + 1, y - 1, bpr)] > 0) &&
          ((img->data[Index(x + 1, y + 0, bpr)]) || (img->data[Index(x + 0, y - 1, bpr)]))) {
          result += 4;
        }
        if (img->data[Index(x + 1, y + 0, bpr)] > 0) {
          result += 8;
        }
        if ((img->data[Index(x + 1, y + 1, bpr)] > 0) &&
          ((img->data[Index(x + 1, y + 0, bpr)]) || (img->data[Index(x + 0, y + 1, bpr)]))) {
          result += 16;
        }
        if (img->data[Index(x + 0, y + 1, bpr)] > 0) {
          result += 32;
        }
        if ((img->data[Index(x - 1, y + 1, bpr)] > 0) &&
          ((img->data[Index(x - 1, y + 0, bpr)]) || (img->data[Index(x + 0, y + 1, bpr)]))) {
          result += 64;
        }
        if (img->data[Index(x - 1, y + 0, bpr)] > 0) {
          result += 128;
        }
      }
      result_data[Index(x, y, bpr)] = result;
    }
  }

  memcpy(img->data, result_data, 4320 * 2160 * 4);
}
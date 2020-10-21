#pragma once

#include "ady_types.h"

/*
  These variables and functions are platform specific but their results are
  used by the Game layer, and so are defined here, but are
  implemented inside the platform layer. For example, the Game is saved and so
  needs to write the save file to disk.
*/
struct dev_ReadFileResult
{
  u64 size;
  void* data;
};

internal dev_ReadFileResult dev_ReadFile(char* filename);
internal void dev_FreeFile(void* memory);
internal b32 dev_WriteFile(char* filename, u32 memorySize, void* memory);

struct ImageData
{
  uchar* data;
  u32 width;
  u32 height;
  i32 size;
  i32 bytesPerRow;
  // i32 format_enum_num; ??? to store dxgi format from LoadImageFromDisk
};

internal i32 LoadImageFromDisk(char* filename, ImageData* imageData);
internal void LoadTextureFromImage();
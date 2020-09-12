/* -------------------- FILE I/O ------------------- */
#pragma region File I/O
internal dev_ReadFileResult dev_ReadFile(char* filename)
{
  dev_ReadFileResult result = { 0 };
  HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (file != INVALID_HANDLE_VALUE)
  {
    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(file, &fileSize))
    {
      Assert(fileSize.QuadPart <= 0xFFFFFFFF);
      result.data = VirtualAlloc(0, fileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      if (result.data)
      {
        DWORD bytesRead;
        if (ReadFile(file, result.data, (i32)fileSize.QuadPart, &bytesRead, 0) && (bytesRead == fileSize.QuadPart))
        {
          result.size = fileSize.QuadPart;
        }
        else
        {
          dev_FreeFile(result.data);
          result.data = 0;
        }
      }
    }
    CloseHandle(file);
  }
  return result;
}

internal void dev_FreeFile(void* memory)
{
  if (memory)
  {
    VirtualFree(memory, 0, MEM_RELEASE);
  }
}

internal b32 dev_WriteFile(char* filename, u32 memorySize, void* memory)
{
  b32 result = false;
  HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
  if (file != INVALID_HANDLE_VALUE)
  {
    DWORD bytesWritten;
    if (WriteFile(file, memory, memorySize, &bytesWritten, 0))
    {
      result = bytesWritten == memorySize;
    }
    else
    {

    }
    CloseHandle(file);
  }
  return result;
}
#pragma endregion
/* -------------------- FILE I/O ------------------- */
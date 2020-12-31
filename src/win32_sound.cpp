#pragma once

global LPDIRECTSOUNDBUFFER win32_SecondarySoundBuffer;

typedef struct win32_SoundStruct {
  i32 bytesPerSample; // = sizeof(i16) * 2;
  i32 samplesPerSecond; // = 48000;
  u32 runningSampleIndex; // = 0;
  i32 wavePeriod; // = samplesPerSecond / toneHertz;
  i32 secondaryBufferSize; // = samplesPerSecond * bytesPerSample;
  i32 latencySampleCount; // samplesPerSecond / 15;
  f32 tsin;
} win32_SoundStruct;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void win32_InitDirectSound(HWND Window, i32 samplesPerSecond, i32 bufferSize) {
  // Load the lib
  HMODULE DirectSoundLibrary = LoadLibrary("dsound.dll");

  if (DirectSoundLibrary) {
    // Get a DirectSound Object
    direct_sound_create* DirectSoundCreate = (direct_sound_create*)
      GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

    LPDIRECTSOUND directSound;
    if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &directSound, 0))) {
      WAVEFORMATEX waveFormat = { 0 };
      waveFormat.wFormatTag = WAVE_FORMAT_PCM;
      waveFormat.nChannels = 2;
      waveFormat.wBitsPerSample = 16;
      waveFormat.nSamplesPerSec = samplesPerSecond;
      waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
      waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
      waveFormat.cbSize = 0;

      if (SUCCEEDED(directSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
        DSBUFFERDESC bufferDescription = { 0 };
        bufferDescription.dwSize = sizeof(bufferDescription);
        bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
        bufferDescription.dwBufferBytes = 0;

        // Create a primary buffer
        LPDIRECTSOUNDBUFFER primaryBuffer;
        if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0))) {
          if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) {
            OutputDebugStringA("Primary Buffer Created Successfully\n");
          }
          else {
            // error
          }
        }
        else {
          // error
        }
      }
      else {
        // error
      }

      // Create a secondary buffer
      DSBUFFERDESC bufferDescription = { 0 };
      bufferDescription.dwSize = sizeof(bufferDescription);
      bufferDescription.dwFlags = 0;
      bufferDescription.dwBufferBytes = bufferSize;
      bufferDescription.lpwfxFormat = &waveFormat;

      if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &win32_SecondarySoundBuffer, 0))) {
        OutputDebugStringA("Secondary Buffer Created Successfully\n");
      }
      else {
        // error
      }
    }
    else {
      // Error
    }
  }
}

internal void win32_FillSoundBuffer(win32_SoundStruct* soundstruct, DWORD byteToLock, DWORD bytesToWrite,
  game_SoundBuffer* soundBuffer) {
  void* region1;
  DWORD r1size;
  void* region2;
  DWORD r2size;
  if (SUCCEEDED(win32_SecondarySoundBuffer->Lock(
    byteToLock, bytesToWrite,
    &region1, &r1size, &region2, &r2size, 0))) {
    // todo assert region sizes are correct
    DWORD region1SampleCount = r1size / soundstruct->bytesPerSample;
    i16* dstSample = (i16*)region1;
    i16* srcSample = soundBuffer->samples;
    for (DWORD sample_index = 0; sample_index < region1SampleCount; sample_index++) {
      *dstSample++ = *srcSample++;
      *dstSample++ = *srcSample++;
      ++soundstruct->runningSampleIndex;
    }

    DWORD region2SampleCount = r2size / soundstruct->bytesPerSample;
    dstSample = (i16*)region2;
    for (DWORD sample_index = 0; sample_index < region2SampleCount; sample_index++) {
      *dstSample++ = *srcSample++;
      *dstSample++ = *srcSample++;
      ++soundstruct->runningSampleIndex;
    }

    win32_SecondarySoundBuffer->Unlock(region1, r1size, region2, r2size);
  }
}

internal void win32_ClearSoundBuffer(win32_SoundStruct* soundstruct) {
  void* region1;
  DWORD r1size;
  void* region2;
  DWORD r2size;
  if (SUCCEEDED(win32_SecondarySoundBuffer->Lock(
    0, soundstruct->secondaryBufferSize,
    &region1, &r1size, &region2, &r2size, 0))) {
    // todo assert region sizes are correct
    u8* dstSample = (u8*)region1;
    for (DWORD byte_index = 0; byte_index < r1size; byte_index++) {
      *dstSample++ = 0;
    }

    dstSample = (u8*)region2;
    for (DWORD byte_index = 0; byte_index < r2size; byte_index++) {
      *dstSample++ = 0;
    }

    win32_SecondarySoundBuffer->Unlock(region1, r1size, region2, r2size);
  }
}
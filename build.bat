@echo off

IF NOT EXIST build mkdir build
pushd build

cl -WX -W4 -wd4100 -wd4201 ^
 -wd4238 ^
 -Oi -GR -EHa- -EHsc ^
 -MT -Gm- ^
 -Zi ..\src\win32_base.cpp ^
 user32.lib ole32.lib gdi32.lib ^
 Xinput.lib d3d12.lib dxgi.lib ^
 dxguid.lib d3dcompiler.lib ^
 ..\libs\pix\WinPixEventRuntime.lib ^
 -nologo

REM cl -WX -W4 -wd4100 -wd4201 ^
REM  -wd4238 ^
REM  -Oi -GR -EHa- -EHsc ^
REM  -O2 -MT -Gm- ^
REM  -Fecarnegie.exe ^
REM  ..\src\win32_base.cpp ^
REM  user32.lib ole32.lib gdi32.lib ^
REM  Xinput.lib d3d12.lib dxgi.lib ^
REM  dxguid.lib d3dcompiler.lib ^
REM  ..\libs\pix\WinPixEventRuntime.lib ^
REM  -nologo

popd
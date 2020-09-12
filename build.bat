@echo off

IF NOT EXIST build mkdir build
pushd build

cl -WX -W4 ^
 -wd4201 -wd4100 ^
 -Oi -GR ^
 -MT -Gm- ^
 -Zi ..\win32_carnegie.c ^
 user32.lib gdi32.lib ^
 Xinput.lib d3d11.lib dxgi.lib ^
 dxguid.lib d3dcompiler.lib -nologo

popd
@echo off

IF NOT EXIST build mkdir build
pushd build

cl -WX -W4 -wd4100 -wd4201 ^
 -wd4238 ^
 -Oi -GR -EHa- -EHsc ^
 -MT -Gm- ^
 -Zi ..\win32_carnegie.cpp ^
 user32.lib ole32.lib gdi32.lib ^
 Xinput.lib d3d12.lib dxgi.lib ^
 dxguid.lib d3dcompiler.lib ^
 ..\libs\pix\WinPixEventRuntime.lib ^
 -nologo

cl -WX -W4 -wd4100 -wd4201 ^
 -wd4238 ^
 -Oi -GR -EHa- -EHsc ^
 -O2 -MT -Gm- ^
 -Fecarnegie.exe ^
 ..\win32_carnegie.cpp ^
 user32.lib ole32.lib gdi32.lib ^
 Xinput.lib d3d12.lib dxgi.lib ^
 dxguid.lib d3dcompiler.lib ^
 ..\libs\pix\WinPixEventRuntime.lib ^
 -nologo

popd
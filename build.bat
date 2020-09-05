@echo off

IF NOT EXIST build mkdir build
pushd build

cl -WX -W4 -wd4100 -wd4201 ^
 -Oi -GR -EHa- ^
 -Zi ..\win32_carnegie.cpp ^
 user32.lib gdi32.lib Xinput.lib ^
 d3d12.lib dxgi.lib dxguid.lib -nologo

popd
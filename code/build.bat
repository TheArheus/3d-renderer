@echo off
if not defined DevEnvDir (
    call vcvarsall x64
)

set LIB_VCPKG="F:\Env\vcpkg\installed\x64-windows\lib"
set INC_VCPKG="F:\Env\vcpkg\installed\x64-windows\include"

set CommonCompileFlags=-MTd -nologo -fp:fast -GS -Gs -EHa -Od -WX- -W4 -Oi -GR- -Gm- -GS -wd4100 -wd4201 -wd4244 -wd4505 -wd4238 -FC -Z7 -I %INC_VCPKG%
set CommonLinkFlags=-opt:ref -incremental:no /NODEFAULTLIB:libcmtd /SUBSYSTEM:CONSOLE /LIBPATH:%LIB_VCPKG%  
if not exist ..\build mkdir ..\build
pushd ..\build

cl %CommonCompileFlags% SDL2main.lib SDL2.lib ..\code\display.cpp -LD /link -opt:ref -incremental:no /LIBPATH:%LIB_VCPKG%
cl %CommonCompileFlags% SDL2main.lib SDL2.lib msvcrt.lib libcmt.lib ..\code\3drenderer.cpp display.obj /link %CommonLinkFlags%
popd

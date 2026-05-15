@echo off

set SDK=third_party\steamworks\sdk

if not exist build mkdir build
copy .\minhook\bin\MinHook.x64.dll .\build

if not exist ".\minhook\bin\MinHook.x64.lib" (
    echo Error: .\minhook\bin\MinHook.x64.lib not found.
    echo Please install it from Github Releases of minhook and extract them into ./minhook
    exit /b 1
)
if not exist ".\minhook\bin\MinHook.x64.dll" (
    echo Error: .\minhook\bin\MinHook.x64.dll not found.
    echo Please install it from Github Releases of minhook and extract them into ./minhook
    exit /b 1
)
if not exist ".\minhook\include\MinHook.h" (
    echo Error: .\minhook\include\MinHook.h not found.
    echo Please install it from Github Releases of minhook and extract them into ./minhook
    exit /b 1
)



cl.exe /LD /MT /O2 /I.\minhook\include /Fe.\build\VERSION.dll .\src\steam_p2p_tweak.cpp /link /DLL "User32.lib" ".\minhook\bin\MinHook.x64.lib"

if errorlevel 1 (
    echo Error: Build failed.
    exit /b 1
)

del .\build\VERSION.exp
del .\build\VERSION.lib


if not exist "./build/steamwebrtc64.dll" (
    echo ** Warning **: steamwebrtc64.dll not found.
    echo It's not required for build, but it's REQUIRED to be put into the game directory.
    echo Please copy it from steam folder to the game directory.
)
echo Now copy VERSION.dll, MinHook.x64.dll, and steamwebrtc64.dll to the game directory!

pause
explorer build
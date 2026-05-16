# SteamP2PDirect

A tiny Windows DLL that tries to force Steam Networking Sockets to strongly prefer direct ICE P2P connections over SDR relays.

Mostly written because debugging broken Steam P2P connectivity is pain.

~~Also, most of this README is AI-generated because I'm lazy.~~

This project is probably pretty hacky and dirty internally. I honestly do not know much about proper Windows injection methods. I mostly only know the old `version.dll` proxy trick. So this is more of a practical debugging tool.

## Known Steam `steamwebrtc64.dll` Issue

There appears to be a long-standing Steam issue affecting some games (for example, Deep Rock Galactic), where `steamwebrtc64.dll` is seemingly not loaded correctly, causing `NAT traversal failed (3999)` errors and preventing direct ICE connections.

This issue has apparently existed for months in some titles. And a surprisingly stupid workaround that often fixes direct P2P connectivity is simply:

**You just copy `steamwebrtc64.dll` into the same folder as the game's `.exe`**, even if the DLL already exists inside the Steam installation directory.

Steam moment.

## Why This Tool Is Still Useful

Even if copying `steamwebrtc64.dll` fixes your problem, this project is still useful for debugging Steam networking issues. The logs can help reveal:

- which ICE candidates were gathered
- which route Steam selected
- whether SDR fallback occurred
- and more

## How It Works

The project builds a fake `VERSION.dll`, which forwards all original exports to the system DLL, hooks `SteamAPI_RunCallbacks`, and applies Steam Networking config tweaks during startup.

## Build Requirements

* Windows x64
* Visual Studio Build Tools (MSVC)
* MinHook

## Setup

1. Install Visual Studio Build Tools:

   https://visualstudio.microsoft.com/visual-cpp-build-tools/

2. Download MinHook releases from:

   https://github.com/TsudaKageyu/minhook

3. Extract it like this:

    ```text
    minhook/
    ├── include/
    │   └── MinHook.h
    └── bin/
        ├── MinHook.x64.dll
        └── MinHook.x64.lib
    ```

## Build

Run build.bat with `x64 Native Tools Command Prompt for VS` under this directory.

## Installation

After building, you must also manually copy the following files:

```text
build/
├── VERSION.dll
└── MinHook.x64.dll
```

from the build directory and

```text
Steam/
├── steamwebrtc64.dll
```

from your Steam installation directory

into the target game directory (e.g. `C:\SteamLibrary\steamapps\common\Deep Rock Galactic\FSD\Binaries\Win64`).

So now these files should be copied into the game folder beside the game executable:

```text
Win64/
├── FSD-Win64-Shipping.exe
├── VERSION.dll
├── MinHook.x64.dll
├── steamwebrtc64.dll
└── (some more dlls)
```

Then launch the game normally through Steam.

## Logging

If debug logging is enabled, logs are written to:

```text
force_ice_steam_dbg.log
```

## Disclaimer

This project does **NOT** magically fix broken networking. If direct P2P still fails, your problem may be:

* broken IPv6
* blocked UDP
* missing `steamwebrtc64.dll`
* cosmic rays

This project is:

* experimental
* hacky
* probably fragile

Use at your own risk.

## Credits

* [MinHook](https://github.com/TsudaKageyu/minhook)
* [Steamworks SDK](https://partner.steamgames.com/doc/sdk)
* [Open-source GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets)
* Helpful LLM assistant
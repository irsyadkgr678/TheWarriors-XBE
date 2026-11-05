# Third-party components

This project stands on other people's work. Nothing here is redistributed as game data; the list below is what the
source tree and the built executables contain, and who wrote it.

## Recompiler, runtime SDK, hook framework

**[XboxXBERecomp](https://github.com/TGP482/XboxXBERecomp)** — TGP482 — MIT
Vendored under [`sdk/`](sdk/). It provides `xbe_recomp`, `xbe_symbols`, the host runtime (kernel HLE, Direct3D 12
backend, XAudio2 audio, DirectSound, XInput/SDL input), the XDK signature data under `sdk/xdk/`, and the CMake glue
(`add_xbe_image`) this project's `CMakeLists.txt` uses.
Its own third-party notices are in [`sdk/THIRD_PARTY.txt`](sdk/THIRD_PARTY.txt) — keep that file with any copy.

**[XenonRecomp](https://github.com/hedge-dev/XenonRecomp)** — hedge-dev — MIT
Overridable function layout and instruction hooks, via the SDK.

## Game-specific hooks, symbol map, and input layer

**[Antriksh1984/TheWarriors-XBERecomp](https://github.com/Antriksh1984/TheWarriors-XBERecomp)** — Antriksh1984
The NTSC port this repository was brought up against. The work taken from it:

- the hand-found symbol addresses in [`config/game.toml`](config/game.toml) (`XPP_GamepadType = 0x40954C`,
  `D3DDevice_FlushVertexCache`, the `IDirectSound*` family) and the `[[function]]`/`[[hook]]` entries;
- [`src/game/kbm.cpp`](src/game/kbm.cpp), [`kbm.h`](src/game/kbm.h), [`camera.cpp`](src/game/camera.cpp),
  [`prompts.cpp`](src/game/prompts.cpp), [`menu.cpp`](src/game/menu.cpp), [`lockpick.cpp`](src/game/lockpick.cpp),
  [`movies.cpp`](src/game/movies.cpp) — keyboard/mouse layer, camera hooks, button prompts, F10 panel;
- [`src/game/dsound.cpp`](src/game/dsound.cpp), [`src/game/d3d.cpp`](src/game/d3d.cpp) and the DirectSound
  implementation in [`sdk/runtime/xdk/dsound.cpp`](sdk/runtime/xdk/dsound.cpp) — the XACT audio path;
- [`src/common/apu_clock.cpp`](src/common/apu_clock.cpp) and the APU sample counter at `0xFE80200C`.

> **Before publishing this repository, confirm with that author.** Their repository does not carry a plain MIT
> license, so redistribution of those files should be agreed with them. The polite and safe route: open an issue or
> send a message describing that the NTSC config and the input/audio hooks are reused here, credit them in the README,
> and keep the link. If they decline, replace those files with your own implementation — the rest of the project
> (launcher, build, docs, runtime fixes) stands on its own.

## Libraries inside the executables

| Component | Author | License |
|---|---|---|
| [Zydis](https://github.com/zyantific/zydis) | Zyantific | MIT |
| [toml++](https://github.com/marzer/tomlplusplus) | Mark Gillard | MIT |
| [Dear ImGui](https://github.com/ocornut/imgui) | Omar Cornut | MIT |
| [SDL3](https://github.com/libsdl-org/SDL) | Sam Lantinga | zlib |
| [LLVM](https://llvm.org/) (clang, lld) | LLVM Project | Apache-2.0 with LLVM exceptions |

The release package carries `compiler/clang.exe`, `compiler/lld-link.exe`, and `compiler/LLVM-LICENSE.txt` so a player
can build the game's code without installing Visual Studio. If you redistribute that folder, keep
`LLVM-LICENSE.txt` beside it.

## The game

*The Warriors* © 2005 Rockstar Games / Rockstar Toronto / Rockstar Leeds, published by Take-Two Interactive.
This project is unofficial, unaffiliated, and not endorsed. It ships no game code, no game data, and no artwork.

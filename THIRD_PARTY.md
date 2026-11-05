# Third-party components

This project builds on other people's work. This file records what is included, who wrote it, and under which license.
No game data is distributed with any of it.

## Recompiler, runtime and hook framework

**[XboxXBERecomp](https://github.com/TGP482/XboxXBERecomp)** — TGP482 — MIT

Vendored under `sdk/`. It provides `xbe_recomp`, `xbe_symbols`, the host runtime (kernel HLE, Direct3D 12 backend,
XAudio2 audio, DirectSound, input), the XDK signature data in `sdk/xdk/`, and the CMake function `add_xbe_image` used
by this project's build. Its own notice file, `sdk/THIRD_PARTY.txt`, must stay with any copy.

**[XenonRecomp](https://github.com/hedge-dev/XenonRecomp)** — hedge-dev — MIT

Overridable function layout and instruction hooks, used through the SDK.

## Game-specific work

**[TheWarriors-XBERecomp](https://github.com/Antriksh1984/TheWarriors-XBERecomp)** — Antriksh1984

The NTSC port this project was brought up against. The following parts originate there:

| File | What it does |
|---|---|
| `config/game.toml` | hand-resolved symbol addresses (`XPP_GamepadType`, `D3DDevice_FlushVertexCache`, the `IDirectSound*` family), the `[[function]]` and `[[hook]]` entries |
| `src/game/kbm.cpp`, `kbm.h` | keyboard and mouse layer: pad mapping, rebinding, mouse-look plumbing |
| `src/game/camera.cpp` | camera hooks for mouse yaw and pitch |
| `src/game/prompts.cpp` | button prompts that show the bound key |
| `src/game/menu.cpp` | F10 settings panel |
| `src/game/lockpick.cpp`, `movies.cpp` | mouse-driven lock pick, skippable boot movies |
| `src/game/dsound.cpp`, `d3d.cpp` | DirectSound and `DrawIndexedVerticesUP` hooks |
| `sdk/runtime/xdk/dsound.cpp` | DirectSound implementation covering XACT's requirements |
| `src/common/apu_clock.cpp` | MCPX sample counter at `0xFE80200C` |

**Permission status:** the upstream repository does not carry a plain MIT license, so these files are included here
with attribution while permission is being confirmed with their author. Replace them with an independent
implementation if that permission is not granted.

## Libraries linked into the executables

| Component | Author | License |
|---|---|---|
| [Zydis](https://github.com/zyantific/zydis) | Zyantific | MIT |
| [toml++](https://github.com/marzer/tomlplusplus) | Mark Gillard | MIT |
| [Dear ImGui](https://github.com/ocornut/imgui) | Omar Cornut | MIT |
| [SDL3](https://github.com/libsdl-org/SDL) | Sam Lantinga | zlib |
| [LLVM](https://llvm.org/) (clang, lld) | LLVM Project | Apache-2.0 with LLVM exceptions |

Release archives include `compiler/clang.exe`, `compiler/lld-link.exe` and `compiler/LLVM-LICENSE.txt` so players can
build the game's code without installing Visual Studio. Keep `LLVM-LICENSE.txt` with that folder if you redistribute
it.

## The game

*The Warriors* © 2005 Rockstar Games / Rockstar Toronto / Rockstar Leeds, published by Take-Two Interactive. This
project is unofficial, unaffiliated with the rights holders, and ships no game code, game data or artwork.

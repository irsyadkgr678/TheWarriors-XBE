# Bring-up notes

Technical record of what was changed to get The Warriors running, and what is still open. Written for whoever
maintains the port next.

Target: `default.xbe`, NTSC/USA retail, SHA-1 `fe246a74501cd10d2ce89ca5dbda5080d2b2edcc`, title ID `0x5454009B`,
XDK 5849.

## 1. Build and run

```
build-here.bat                     -> out/build/release/TheWarriors.exe, .dll, TheWarriorsLauncher.exe
Play.bat                           -> starts the launcher
assets/                            -> ISO or fully extracted game files
```

A full recompile of the game code takes about 3 minutes here (128 partitions). The recompiler reports 179 of 243
functions and 10 of 12 data symbols resolved by signature; the remainder is listed in section 5.

## 2. Patches carried in this repository

| Location | Change |
|---|---|
| `config/game.toml` | 19 symbol addresses found by hand, one `[[function]]` entry (`0x35637D`), three `[[hook]]` entries for camera yaw/pitch and the text parser's button glyph |
| `sdk/runtime/xdk/dsound.cpp` | replaced with an implementation that models XACT: mix-in buffers, 3D voice data, envelopes, stream pause/flush |
| `src/game/dsound.cpp` | hooks the console-side DirectSound methods so they never run against host objects |
| `src/game/d3d.cpp` | `D3DDevice_DrawIndexedVerticesUP`, which the SDK has no version of |
| `src/game/kbm.cpp`, `kbm.h`, `camera.cpp`, `prompts.cpp`, `menu.cpp`, `lockpick.cpp`, `movies.cpp` | keyboard and mouse layer, camera hooks, prompts, F10 panel |
| `src/common/apu_clock.cpp` | commits and feeds the MCPX counter at `0xFE80200C`, which XACT reads directly |
| `src/common/game_info.cpp` | wires every host hook: load/save, window, message, UI, pad, rumble, and the APU clock |
| `build.bat` | copies the matching MSVC runtime DLLs next to the executable |
| `sdk/runtime/kernel/{objects,threads}.cpp`, `kernel.h` | reverted to the previous waiter implementation (section 4) |

## 3. Faults found during bring-up

**Missing assets look like a code bug.** With only `default.xbe` present, the runtime selects that folder instead of
mounting the ISO, the game fails to find its WAD files, and faults about 0.4 s after boot with
`C0000005 write ... guest 00000000`. The same crash appears if an ISO is present but a stray `default.xbe` sits beside
it. Fix: ISO only, or a complete extraction. This cost the most time; it is documented in the README.

**CRT mismatch.** The runtime is built against the toolset's STL headers (14.44), while the system `MSVCP140.dll` was
14.28. The executable faulted inside `MSVCP140.dll` before writing a log. `build.bat` now copies the redistributable
DLLs next to the executable, as the SDK's own releases do.

**Unknown indirect call at boot.** The analysis missed `0x35637D`; the trap message names it. Adding
`[[function]] at = 0x35637D` and rebuilding resolves it.

**XACT reading the APU counter.** With audio initialised, the game faulted reading `0xFE80200C`. That page is not
committed by default. `apu_clock.cpp` commits it and keeps a 48 kHz counter on a host thread; `game_info.cpp` runs it
from the `loaded` hook.

**Console DirectSound running on host objects.** Resolving the `IDirectSound*` symbols without hooks made the guest
execute Microsoft's DirectSound code against host buffers, which read a bad pointer and faulted. The hooks in
`src/game/dsound.cpp` intercept those methods and forward to the host implementation.

## 4. SDK regression in the waiter implementation

The newer XboxXBERecomp runtime replaces the global condition variable with per-object waiter lists (`Waiter`,
`SignalChanged`, `Wake`). With that code the game stops immediately after the `D3D12 adapter` log line: no error, no
frame, no further log output. Swapping in `kernel/objects.cpp`, `kernel/threads.cpp` and `kernel/kernel.h` from the
port this project follows restores normal boot (`dsound: effects image` at ~18 s, input active at ~21 s). This is a
runtime regression for this title, not a configuration problem. Re-test before updating the SDK.

## 5. Remaining trap stubs

`xbe_symbols` reports these unresolved; they trap if called:

- Bink: `BinkDoFrame`, `BinkGetSummary`, `BinkNextFrame`, `BinkWait`. The title's own Bink code is in-image, so
  playback works.
- DirectSound: `IDirectSoundBuffer_SetOutputBuffer`, `SetPlayRegion`, `SetRolloffCurve`, `SetAllParameters`,
  `SetFilter`, `SetEG`, `Pause`, `StopEx`, `IDirectSoundStream_SetEG`, `IDirectSound_CommitEffectData`,
  `DownloadEffectsImage`, `GetEffectData`, `GetOutputLevels`, `SynchPlayback`, `CDirectSoundStream_AddRef/Release`.
  Most are covered by the host implementation; the rest are unused in single player.
- Networking: Winsock and XNet/XOnline exports, so System Link is not available.

## 6. Open work

- MSAA: the backend has no multisampled screen targets. Reworking the screen-sized targets is the prerequisite.
- 720p: the SDK fork this project follows adds `app::Hdtv720p()` and reports 720p to the title. Not ported yet, so the
  game stays at 480p.
- System Link: needs the Winsock/XNet layer.
- Long-run stability: tested through the opening missions only.

## 7. Measurements

Ryzen 5 150, integrated Radeon, 15.2 GB RAM:

| Metric | Value |
|---|---|
| Resident memory, menu | ~400 MB |
| CPU, menu (60 FPS cap) | ~58% of one core |
| Threads | 30 |
| Recompiled game code | 129 translation units, ~15 MB DLL |
| First-launch compile | 1.5–4 minutes |
| Release archive | 59 MB (mostly clang and lld) |

## 8. Credits

XboxXBERecomp by TGP482 (MIT) provides the recompiler and runtime. The symbol map, the DirectSound/D3D/APU hooks and
the keyboard/mouse layer come from Antriksh1984's NTSC port; see [THIRD_PARTY.md](THIRD_PARTY.md) for the permission
status.

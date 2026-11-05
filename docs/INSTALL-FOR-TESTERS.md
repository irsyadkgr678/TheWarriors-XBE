# Tester guide

Written for someone who has never seen this project. No compiler, no GitHub account, no internet needed after the
download finishes.

## What you need

1. **Windows 10 or 11, 64-bit.** 32-bit Windows will not work.
2. **A GPU with Direct3D 12 support.** Roughly NVIDIA GTX 600 or newer, AMD HD 7000 or newer, or Intel HD 4200
   (Haswell) or newer. Quick check: press `Win+R`, type `dxdiag`, open the **Display** tab and look at **Feature
   Levels**. It must list `11_0`, `11_1`, `12_0` or `12_1`.
3. **Your own Xbox NTSC/USA disc**, as a single `.iso` file. The PS2 and PSP versions are not supported. The image is
   about 5 GB and does not need to be extracted.

## Installation

1. **Download** `TheWarriors-port-v0.1.0-alpha.zip` (about 59 MB).
2. **Extract** it to a new folder you can write to, for example `D:\Games\TheWarriors`. Avoid `C:\Program Files`, and
   do not run anything from inside the archive.
3. **Put the ISO** in the `assets` folder. The result looks like this:

   ```
   TheWarriors\
     TheWarriorsLauncher.exe
     TheWarriors.exe
     compiler\
     assets\
       TheWarriors.iso      <- your ISO, any filename
   ```

4. **Run `TheWarriorsLauncher.exe`.**
   If the ISO is somewhere else, open the **Game files** tab, point it at that folder, and use the **Graphics** tab if
   you want to change resolution or render scale.
5. **Press Play.**
   The first launch compiles the game's code from your ISO. A progress window appears; it takes about 2–4 minutes
   depending on the CPU. This happens once per release, after which the game starts directly.
6. In game, press **F10** for display settings, mouse sensitivity, key bindings and controller options.

## Default controls

| Action | Key |
|---|---|
| Move | W A S D |
| Camera | mouse |
| Light / heavy attack | left mouse / right mouse |
| Grab / throw | E or Backspace |
| Action (jump, climb) | Space |
| Sprint | Left Shift |
| Pause | Esc |

Everything can be rebound in the launcher or in the F10 panel.

## If it does not work

| What you see | What it means |
|---|---|
| `Put the game in the assets folder` | No ISO found: it is still inside an archive, or the folder is one level too deep |
| `...is a different release of the game` | The ISO is not the NTSC/USA retail release; this build does not support other regions |
| Window closes about 0.4 s after opening | `assets` contains a loose `default.xbe` without the WAD files. Delete it and keep the ISO only |
| The first compile fails | Antivirus quarantined `compiler\clang.exe` or `lld-link.exe`. Restore them and exclude the game folder |
| Black screen | Update the GPU driver. On dual-GPU laptops, force `TheWarriors.exe` onto the discrete GPU |
| No sound | Set the intended playback device as the Windows default before starting |

## Reporting a problem

Send these three things:

1. `logs/TheWarriors.log` (if it crashed, the last 50 lines are enough).
2. The newest file in `CrashDumps/`, if there is one.
3. CPU, GPU and RAM.

Do not attach the ISO, game files, or `TheWarriors.dll`.

## What to expect

This is an alpha build. Working: boot, menus, gameplay, Direct3D 12 rendering, audio, mouse and keyboard, gamepads.
Not implemented: MSAA, 720p output, System Link. Later missions are untested, so crashes during gameplay are still
possible.

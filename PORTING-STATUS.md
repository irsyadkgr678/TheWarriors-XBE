# PORTING STATUS — The Warriors (Xbox NTSC) → PC

Status nyata per build terakhir: game **boot, inisialisasi audio, dan mouse+keyboard bekerja**.
XBE: `default.xbe` NTSC/USA, SHA-1 `fe246a74501cd10d2ce89ca5dbda5080d2b2edcc`.

## 1. Cara menjalankan (penting)

`assets\` harus **lengkap**:

- **Opsi A (paling aman)** — taruh **hanya** ISO disc Xbox di `assets\`. Runtime me-mount ISO dan membaca semuanya
  dari situ.
- **Opsi B** — ekstrak seluruh isi ISO ke `assets\` (`default.xbe`, `audio\`, `bik\`, `Main.wad0..3`, `Paks.wad0..2`,
  `System.wad0`, `XBoxWad.idx`).

> **Jangan** hanya menaruh `default.xbe` sendirian di `assets\` sementara file WAD-nya tidak ada. Runtime memilih
> folder (bukan ISO) begitu `default.xbe` ada di sana, lalu game tidak menemukan datanya dan mati dengan
> `C0000005 write ... guest 00000000` sekitar 0,4 detik setelah boot. Ini penyebab crash yang paling menjebak saat
> bring-up: crash-nya tampak seperti bug recompiler, padahal isi folder yang tidak lengkap.

Keduanya boleh ada sekaligus selama foldernya lengkap (folder yang dipakai lebih dulu).

## 2. Build

```bat
build-here.bat
```

Memakai `toolchain\` portabel (git/cmake/ninja) + Visual Studio 2022 Build Tools (clang 19, Windows SDK 10.0.26100).
Hasil: `out\build\release\TheWarriors.exe` + `TheWarriors.dll`. Recompile penuh ~3 menit di mesin ini (128 partisi).

## 3. Tambalan yang ada di repo ini

| Bagian | Isi |
|---|---|
| `config/game.toml` | 19 simbol hasil temuan manual (`XPP_GamepadType = 0x40954C`, `D3DDevice_FlushVertexCache`, keluarga `IDirectSound*`), 1 `[[function]]` (`0x35637D`), 3 `[[hook]]` (kamera yaw/pitch, prompt glyph) |
| `sdk/runtime/xdk/dsound.cpp` | lapisan DirectSound yang paham XACT (mix-in buffer, 3D voice data, envelope, stream pause/flush). Asli: `dsound.cpp.sdk-orig` |
| `src/game/dsound.cpp` | hook method DirectSound konsol → objek host |
| `src/game/d3d.cpp` | hook `D3DDevice_DrawIndexedVerticesUP` |
| `src/game/kbm.cpp`, `kbm.h` | **mouse + keyboard**: tombol pad dipetakan ke key, mouse memutar kamera sendiri, rebind, sensitivitas |
| `src/game/camera.cpp` | hook yaw/pitch stick kanan → mouse |
| `src/game/prompts.cpp` | prompt tombol menampilkan key yang terpasang |
| `src/game/lockpick.cpp`, `movies.cpp` | lock pick memakai mouse, logo boot bisa di-skip |
| `src/game/menu.cpp` | panel **F10** (display, sensitivitas, rebind key, controller). Baris MSAA dihapus — renderer kita belum punya MSAA |
| `src/common/apu_clock.cpp` | penghitung sampel MCPX di `0xFE80200C` yang dibaca XACT |
| `src/common/game_info.cpp` | semua hook host disambungkan (load/save/window/message/ui/pad/rumble) |
| `build.bat` | menyalin `msvcp140/vcruntime140*.dll` ke samping exe (STL 14.44 vs DLL sistem 14.28 = crash saat start) |
| `sdk/runtime/kernel/{objects,threads}.cpp`, `kernel.h` | dikembalikan ke versi SDK yang terbukti boot (lihat bagian 4) |
| `launcher/main.cpp` | **TheWarriorsLauncher.exe**: tab Grafik / Controller / Keyboard & Mouse / Game files, pemilih bahasa ID/EN, watermark, tombol PLAY |
| `sdk/runtime/app/{app.h,settings.cpp}` + `game_files.cpp` | key `game_files` & `game_exe`; folder data game bisa di drive mana pun |
| `Play.bat` | membuka launcher (fallback ke game langsung bila launcher tidak ada) |

## 3b. Launcher dan pemisahan game files

Launcher adalah executable terpisah (`TheWarriorsLauncher.exe`, ImGui + Direct3D 11) yang **menulis `settings.toml`
yang sama** dengan yang dibaca game, lalu `CreateProcess` ke `TheWarriors.exe`. Tidak ada logika game di dalamnya, jadi
kedua jalur (launcher atau F10 in-game) selalu konsisten.

Key baru di `settings.toml`:

| Key | Arti |
|---|---|
| `game_files` | folder data game, absolut atau relatif terhadap exe. Kosong = `assets\` di samping/di atas exe seperti biasa |
| `game_exe` | catatan launcher soal executable yang dijalankan (game mengabaikannya) |

Urutan muat di `main.cpp`: `app::Load()` → `kGame.init()` → `FindGame()`; jadi `game_files` sudah terbaca sebelum
pencarian aset. Sudah diuji: dengan `game_files` menunjuk folder lain (junction ke `assets`), log menulis
`Game: default.xbe in D:\...\_testdata, sha1 fe246a74…` dan game boot normal.

Perilaku kalau folder yang ditunjuk tidak valid: `FindAssets()` menulis peringatan lalu jatuh kembali ke pencarian
`assets\` biasa, jadi game tidak langsung mati.

Launcher diuji dengan `--selftest` (baca `settings.toml`, tulis ulang, keluar tanpa jendela): seluruh key
`[controller]`, `[keys]`, `[mouse]`, dan nilai grafik tetap utuh setelah round-trip.

Bahasa antarmuka: pemilih **Indonesia / English** di kanan atas, disimpan sebagai `launcher_language` (`id`/`en`).
Teks UI diambil dari tabel `kTr[]` di `launcher/main.cpp`, jadi menambah bahasa ketiga cukup menambah satu kolom.
Diuji dengan tangkapan layar: baris kredit watermark berubah lebar 480 px (ID) → 444 px (EN), artinya pergantian
bahasa benar-benar terjadi saat runtime, bukan hanya tersimpan.

Watermark: judul "THE WARRIORS" samar di tengah (alpha 46) + baris kredit versi di kanan bawah, digambar ke draw list
jendela panel tepat sebelum `End()`. Catatan: `GetForegroundDrawList()` dan overlay window terpisah **tidak** tampil
kokoh di sini (tertutup jendela panel), jadi jangan dikembalikan ke cara itu tanpa diuji ulang.

## 4. Regresi SDK yang menggantungkan boot

SDK TGP482 yang lebih baru menulis ulang penungguan objek kernel (`Waiter`, daftar tunggu per objek, `SignalChanged`).
Dengan versi itu, The Warriors **berhenti tepat setelah `D3D12 adapter`**: tanpa error, tanpa progres, tanpa audio.
Mengganti `kernel/objects.cpp`, `kernel/threads.cpp`, dan `kernel/kernel.h` dengan versi fork upstream membuat boot
lanjut normal (`dsound: effects image` ~18 s, `kbm` aktif ~21 s). Ini regresi, bukan kesalahan konfigurasi game.

## 5. Kontrol default

| Aksi | Key |
|---|---|
| Jalan | W A S D, Left Alt = walk |
| Kamera | mouse |
| Serang ringan (X) / berat (A) | Klik kiri / klik kanan, Enter |
| Grab (B), aksi (Y) | E / Backspace, Space |
| Sprint (LT), target (RT) | Left Shift, Q |
| Block (Black), White | F, R |
| Rage | Z |
| Pause / Back | Esc / Tab |
| Panel setting | F10 |

Semua bisa di-rebind di F10; pengaturan tersimpan di `settings.toml` (bagian `[keys]`, `[mouse]`, `[controller]`).

## 6. Sisa pekerjaan

- MSAA belum ada (renderer lawas; baris di menu sengaja dihilangkan).
- System Link (jaringan) belum: simbol Winsock/XNet masih trap, tidak dipakai single player.
- 720p: `app::Hdtv720p()` belum di-porting, jadi game memakai 480p seperti TV standar.
- Belum diuji menyeluruh: hanya boot + menu + beberapa menit berjalan.

## 7. Kredit

- [XboxXBERecomp](https://github.com/TGP482/XboxXBERecomp) — recompiler + runtime (MIT).
- [Antriksh1984/TheWarriors-XBERecomp](https://github.com/Antriksh1984/TheWarriors-XBERecomp) — acuan alamat simbol,
  hook DirectSound/D3D, APU clock, dan lapisan `kbm`/`camera`/`prompts`/`menu` untuk XBE NTSC yang sama.
- XenonRecomp, Zydis, toml++, Dear ImGui, SDL3, LLVM.

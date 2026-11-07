# The Warriors — build PC native (recompilasi statis dari Xbox)

![status](https://img.shields.io/badge/status-alpha-orange)
![platform](https://img.shields.io/badge/platform-Windows%2010%2F11%20x64-blue)
[![Download](https://img.shields.io/badge/download-v0.1.0--alpha-brightgreen)](https://github.com/irsyadkgr678/TheWarriors-XBE/releases/tag/v0.1.0-alpha)

Port tidak resmi *The Warriors* (Xbox, 2005). Kode x86 di dalam `default.xbe` diterjemahkan ke C++ saat build, lalu
dijalankan sebagai aplikasi Windows 64-bit: Direct3D 12 untuk gambar, XAudio2 untuk suara, SDL3 untuk window dan
input. Tidak ada emulator di dalamnya.

> Repositori ini tidak berisi kode game, data game, maupun aset. Untuk build dan bermain, kamu perlu disc Xbox
> NTSC/USA milik sendiri.

Dokumentasi lengkap (Inggris): [README.md](README.md)
Panduan langkah demi langkah untuk penguji (tanpa compiler): [docs/INSTALL-FOR-TESTERS.id.md](docs/INSTALL-FOR-TESTERS.id.md)

## Status

Sudah jalan: boot ke menu dan masuk gameplay, renderer Direct3D 12, audio XACT (dialog, musik, efek 3D), mouse dan
keyboard, gamepad lewat SDL3, panel setting F10.

Belum ada: MSAA, output 720p (masih 480p), System Link. Build ini alpha dan baru diuji sampai misi-misi awal.

## Syarat

| | Minimum |
|---|---|
| OS | Windows 10 64-bit |
| CPU | x86-64 dengan SSE4.2 dan POPCNT (Intel Nehalem/Core generasi 1 ke atas, AMD FX ke atas), 2 core |
| GPU | Direct3D 12 feature level 11_0 (GTX 600+, HD 7000+, Intel HD 4200+) |
| RAM | 2 GB |
| Disk | sekitar 7 GB untuk data game |

Core 2 Duo, Pentium Dual-Core dan Phenom II tidak bisa: CPU-nya belum punya SSE4.2. GPU yang punya feature level
11_0 tapi tanpa driver D3D12 (misalnya Intel HD 4000 Ivy Bridge) juga tidak bisa menjalankan game, walaupun launcher
tetap terbuka.

## Unduhan

Rilis terbaru: <https://github.com/irsyadkgr678/TheWarriors-XBE/releases/tag/v0.1.0-alpha>

## Cara main

1. Ekstrak rilis ke folder yang bisa ditulis. Jangan di `C:\Program Files`, dan hindari folder yang disinkronkan
   OneDrive.
2. Taruh data game di `assets/`, salah satu bentuk:
   - satu file `.iso` Xbox (nama bebas), atau
   - hasil ekstrak lengkap: `default.xbe`, `Main.wad0..3`, `Paks.wad0..2`, `System.wad0`, `XBoxWad.idx`, `audio/`,
     `bik/`.
3. Jalankan `TheWarriorsLauncher.exe`. Kalau data game tidak ada di `assets/`, buka tab **Folder game** dan arahkan
   ke folder atau ISO-nya. Tekan **MAINKAN**.
4. Start pertama mengompilasi kode game dari `default.xbe` milikmu. Sekitar 1,5–4 menit, sekali per rilis.
5. Tekan **F10** di dalam game untuk pengaturan display, mouse, key binding dan controller.

Jangan menaruh `default.xbe` sendirian tanpa file WAD. Runtime akan memakai folder itu dan game berhenti sekitar
0,4 detik setelah boot dengan pesan `C0000005 write ... guest 00000000`.

## Data game

- Hanya `.iso` yang dibaca. File `.7z`, `.zip`, `.rar`, `.cso` atau ISO terpecah harus diekstrak atau digabung dulu.
- `default.xbe` harus rilis **NTSC/USA retail**, SHA-1 `fe246a74501cd10d2ce89ca5dbda5080d2b2edcc`. Versi PAL/EUR
  memunculkan peringatan dan hampir pasti crash. Versi PS2/PSP tidak kompatibel.
- Folder data boleh di drive mana pun. Pilih lewat launcher; nilainya disimpan sebagai `game_files` di
  `settings.toml`. Kalau kosong, runtime mencari `assets/` di samping exe dan di folder di atasnya.

## Launcher

Tab **Grafik**: mode layar, resolusi, render scale, FPS cap, V-Sync, anisotropic filtering, tekstur pengganti.
Tab **Controller**: kecepatan kamera, deadzone, invert, getaran, remap tombol.
Tab **Keyboard & Mouse**: sensitivitas, smoothing, invert Y, klik menu, seluruh binding.
Tab **Folder game**: folder data, executable game, lokasi `settings.toml`.

Antarmuka tersedia dalam Indonesia dan Inggris. Launcher dan game membaca `settings.toml` yang sama.

Launcher menggambar latarnya sendiri. Kalau ada `background.png` atau `background.jpg` di samping exe, gambar itu yang
dipakai dan diskalakan menutupi jendela. File tersebut milikmu; tidak ada artwork yang disertakan di repo ini dan
namanya sudah di-ignore oleh git.

## Kontrol default

| Aksi | Key |
|---|---|
| Jalan | W A S D, Left Alt untuk jalan pelan |
| Kamera | mouse |
| Serang ringan / berat | klik kiri / klik kanan, Enter |
| Grab / lempar | E, Backspace |
| Aksi (lompat, panjat) | Space |
| Sprint / target | Left Shift / Q |
| Block / White | F / R |
| Rage | Z |
| D-pad | panah, 1 2 3 4 |
| Pause / panel setting | Esc / F10 |

Semua bisa diubah di launcher atau di panel F10.

## Masalah yang sering muncul

| Gejala | Penyebab |
|---|---|
| Keluar langsung, fault di `MSVCP140.dll` | DLL CRT sistem lebih lama dari toolset. Pakai DLL yang disertakan di rilis, taruh di samping exe. |
| `Put the game in the assets folder` | `assets/` kosong, ISO masih ter-zip, atau ekstrak satu folder terlalu dalam. |
| `...is a different release of the game` | XBE bukan NTSC retail; butuh build lain. |
| Crash ~0,4 detik setelah boot | folder data tidak lengkap, biasanya hanya `default.xbe`. |
| Kompilasi start pertama gagal | antivirus mengarantina `compiler/clang.exe` atau `lld-link.exe`. |
| Layar hitam | perbarui driver GPU; di laptop dua GPU, paksa exe ke GPU diskrit. |
| FPS rendah | turunkan `render_percent` ke 60–80 dan set `fps_cap` 30. |

Log di `logs/TheWarriors.log`, save di `hdd/`, setelan di `settings.toml`, crash dump di `CrashDumps/`, semuanya di
samping exe.

## Build dari source

Butuh Visual Studio 2022 Build Tools (C++ Clang tools, C++ CMake tools, Windows SDK), Git, CMake 3.25+, Ninja, dan
`default.xbe` milikmu di `assets/`.

```bat
build-here.bat
```

Hasil di `out/build/release/`: `TheWarriors.exe`, `TheWarriors.dll`, `TheWarriorsLauncher.exe`, dan folder
`compiler/`. Paket rilis dibuat dengan `package.bat` (tanpa alat tambahan) atau `release.bat` (butuh 7-Zip).

## Lisensi

Kode asli repositori ini (launcher, skrip build, dokumentasi, perbaikan host-side) dirilis MIT; lihat
[LICENSE](LICENSE). Lisensi itu tidak memberi hak atas game, kode game, atau asetnya. Komponen pihak ketiga dan asal
hook game ada di [THIRD_PARTY.md](THIRD_PARTY.md).

*The Warriors* © Rockstar Games / Take-Two Interactive. Proyek ini tidak resmi dan tidak berafiliasi.

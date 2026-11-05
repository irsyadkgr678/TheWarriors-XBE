# Android notes (historical)

> Catatan dari percobaan awal memindahkan proyek ini ke Android. Bukan bagian dari port PC dan tidak lagi
> dipelihara. Port PC ada di [README.md](../README.md); catatan teknisnya di
> [PORTING-STATUS.md](../PORTING-STATUS.md).

Ditulis untuk pemula. Baca bagian 1 dulu, karena ada satu kenyataan teknis yang menentukan semuanya.

---

## 1. Kenyataan penting: tool ini TIDAK bisa membuat game Android

XboxXBERecomp bekerja dengan menerjemahkan **`default.xbe`** — file executable game **Xbox original (2001)** yang berisi kode **x86 32-bit** — menjadi **`.exe` Windows 64-bit**. Runtime-nya memakai **Direct3D 12** dan **XAudio2**, plus `d3d12.lib`, `xaudio2.lib`, `dxgi.lib` (lihat `sdk/cmake/XbeRecomp.cmake` baris 120). README template-nya sendiri menulis syarat: *Windows 10 64-bit, GPU dengan Direct3D 12*.

Artinya:

| Yang kamu harap | Kenyataannya |
|---|---|
| Hasil berupa `.apk` | Hasilnya `TheWarriors.exe` (Windows x64) |
| Jalan langsung di Android | Butuh lapisan kompatibilitas (Winlator) — eksperimental |
| Tool membaca APK | Tool hanya membaca `.xbe` |

Game Android berbentuk **APK berisi kode ARM**, tidak punya XBE sama sekali. Jadi tidak ada jalur "APK → XboxXBERecomp". Membuat runtime ini jalan native di Android berarti menulis ulang seluruh renderer D3D12 → Vulkan, XAudio2 → AAudio, dan layer kernel-nya: itu proyek bertahun-tahun, bukan pekerjaan pemula.

**Jalur yang realistis** untuk The Warriors di Android ada di bagian 5.

---

## 2. Sudah disiapkan di PC kamu

Semua tanpa hak admin, di folder `toolchain\`:

| Tool | Versi | Status |
|---|---|---|
| Git (PortableGit) | 2.56.0 | ✅ jalan, sudah diuji `git clone` |
| CMake | 4.4.4 | ✅ (syarat proyek ≥ 3.25) |
| Ninja | 1.13.2 | ✅ |
| Python | 3.13 | ✅ (sudah ada sebelumnya, untuk `sdk/tools/symbols/xbe2pe.py`) |

Kerangka proyek juga sudah dibuat di `TheWarriors-XBE\`:

```
TheWarriors-XBE\
├─ sdk\                 <- XboxXBERecomp (SDK)
├─ assets\              <- TARUH FILE GAME DI SINI (masih kosong)
├─ config\game.toml     <- dibuat ulang otomatis oleh xbe_symbols
├─ src\                 <- hook game (nanti diisi kalau ada bug)
├─ build.bat            <- build asli (sudah diisi The Warriors / TheWarriors)
├─ build-here.bat       <- pembungkus: atur PATH toolchain lalu panggil build.bat
├─ setup-env.ps1        <- untuk kerja manual di PowerShell
└─ release.bat          <- bikin paket zip rilis
```

---

## 3. Dua hal yang masih kurang (blocker)

### a. Visual Studio + Windows SDK — **wajib, butuh admin**
Proyek ini butuh `clang++` dengan ABI MSVC, `lld-link`, header/lib Direct3D 12 dan XAudio2. Itu hanya ada kalau Visual Studio terpasang. `build.bat` sendiri akan menawarkan memasang komponen yang kurang, tapi VS dasarnya harus ada.

- Unduh: **Visual Studio 2022 Community** (gratis) → <https://visualstudio.microsoft.com/downloads/>
- Saat instalasi centang **"Desktop development with C++"**
- Komponen tambahan yang harus ada (bisa ditambah nanti lewat *Visual Studio Installer → Modify → Individual components*):
  - **C++ Clang tools for Windows** (`Microsoft.VisualStudio.Component.VC.Llvm.Clang`)
  - **C++ Clang-cl for v143 build tools** (`...VC.Llvm.ClangToolset`)
  - **C++ CMake tools for Windows** (`...VC.CMake.Project`)
- Ukuran ± 4–7 GB.

Saya tidak bisa memasangnya untukmu dari sini karena butuh hak administrator (akun kamu sekarang: non-admin) dan unduhannya besar. Kalau kamu mau, saya bisa memandu klik per klik.

### b. `default.xbe` The Warriors — **wajib, dari disc milikmu**
Proyek ini **tidak** menyertakan aset game, dan saya tidak akan mengarahkan ke sumber ilegal. Yang dibutuhkan:

```
assets\
├─ default.xbe          <- dari disc Xbox The Warriors milikmu
├─ (folder game: audio\, movies\, ...)
```

Boleh juga berupa **ISO disc utuh** yang diletakkan di `assets\` (runtime bisa membacanya). Cara paling sah: rip disc Xbox milikmu sendiri.

---

## 4. Langkah build (setelah 3a & 3b beres)

1. Salin file game ke `assets\`.
2. Dobel-klik **`build-here.bat`**.
3. Skrip akan:
   - mencari Visual Studio,
   - menjalankan `xbe_symbols` untuk memindai XBE dan menulis `config\game.toml`,
   - **recompile kode game: 30–90 menit** pada build pertama,
   - menghasilkan `out\build\release\TheWarriors.exe`.
4. Uji di PC dulu: `out\build\release\TheWarriors.exe`.

Kalau muncul `missing: clang++ cmake ninja git` → jalankan lewat `build-here.bat`, bukan `build.bat`.
Kalau muncul `Visual Studio with the C++ workload was not found` → bagian 3a belum selesai.
Kalau muncul `Put the full game files in the assets folder first` → bagian 3b belum selesai.

---

## 5. Jalur ke Android — pilih salah satu

### Opsi A — Winlator (pakai hasil build Xbox di atas) ⚠️ eksperimental
Winlator menjalankan aplikasi Windows x64 di Android lewat Box64 + Wine, dan D3D12 diterjemahkan ke Vulkan lewat vkd3d-proton.

- Salin folder `out\build\release\` (berisi `TheWarriors.exe`, folder `compiler\`, `assets\`) ke HP.
- Buat container Winlator: Windows 10, Box64 performa, DXVK **dan vkd3d** aktif, driver Turnip untuk GPU Adreno.
- Kenapa berisiko: game ini butuh D3D12 feature level yang layak, memakai delay-load DLL, dan controller. vkd3d-proton di Android masih terbatas; hasilnya bisa crash atau lambat.
- Syarat praktis: Snapdragon 8-series (atau Adreno 7xx ke atas) + RAM 8 GB+.

### Opsi B — PPSSPP (versi PSP) ✅ paling mudah
The Warriors punya versi PSP. PPSSPP jalan sangat baik di Android kelas menengah, bisa upscale resolusi, dan kontrolnya sudah dipetakan. Ini jalur yang saya rekomendasikan kalau tujuanmu **bermain di HP**, bukan riset recompiler.
- Butuh: ISO PSP The Warriors dari disc/UMD milikmu.

### Opsi C — NetherSX2 / AetherSX2 (versi PS2) ✅ kualitas lebih tinggi
Versi PS2 lebih kaya aset daripada PSP. Butuh HP yang lebih kencang (Snapdragon 870+ ideal).
- Butuh: ISO PS2 milikmu.

### Opsi D — Streaming
Build PC-nya, lalu stream ke HP dengan Moonlight/Sunshine. Kualitas paling bagus, tanpa masalah kompatibilitas, tapi butuh jaringan bagus.

---

## 6. Catatan teknis yang saya temukan saat setup

- **`git` gagal dengan `schannel: AcquireCredentialsHandle failed: SEC_E_NO_CREDENTIALS`** — ini terjadi karena sandbox pengembangan di sesi ini memblokir akses kredensial TLS Windows. Solusinya: `http.sslBackend=openssl` (sudah otomatis lewat `GIT_CONFIG_GLOBAL` di `build-here.bat`). Di Windows biasa masalah ini tidak muncul.
- **`winget` rusak** di sesi ini (exit `APPINSTALLER_CLI_ERROR_INTERNAL_ERROR`) — makanya toolchain dipasang portabel, bukan lewat winget/choco.
- **`release.bat` butuh 7-Zip** (`7z` di PATH) untuk membuat zip rilis. Belum terpasang; hanya perlu kalau kamu mau membagikan build.

---

## 7. Ringkasan keputusan yang perlu kamu ambil

1. **Mau main The Warriors di HP** → pakai **PPSSPP** (opsi B). Selesai dalam 15 menit, tanpa Visual Studio, tanpa recompile 90 menit.
2. **Mau benar-benar mencoba jalur recompiler Xbox → Windows → Winlator** → selesaikan 3a + 3b, lalu saya bantu iterasi build/Winlator-nya.
3. **Mau belajar recompiler-nya saja** (tanpa hasil Android) → sama seperti nomor 2.

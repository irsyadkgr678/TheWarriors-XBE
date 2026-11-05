# The Warriors — Xbox → PC (static recompilation)

Port PC tidak resmi dari **The Warriors (Xbox original, 2005)**: kode x86 di dalam `default.xbe` diterjemahkan ke C++
saat build, lalu dijalankan sebagai aplikasi Windows 64-bit native — **bukan emulator**. Gambar lewat Direct3D 12,
suara lewat XAudio2, input lewat SDL3, plus launcher terpisah untuk grafik/controller/keyboard dan pemilihan folder
data game.

> **Tidak ada data game di repositori ini.** Kamu harus memakai salinan Xbox NTSC/USA milikmu sendiri.

<details>
<summary><b>English summary</b></summary>

An unofficial **static recompilation** of *The Warriors* (original Xbox, 2005) to a native 64-bit Windows executable.
The game's x86 code is translated ahead of time from your own `default.xbe`; nothing is emulated at runtime. The
renderer is Direct3D 12, audio XAudio2, input SDL3. A separate launcher writes graphics/controller/keyboard settings
and points the game at a data folder anywhere on disk. **Bring your own NTSC/USA Xbox dump** (disc image or extracted
files); no game data is distributed. Alpha quality: it boots and plays, with rough edges listed below.

</details>

---

## 1. Status jujur

| Bagian | Keadaan |
|---|---|
| Boot dari `default.xbe` → menu | ✅ |
| Renderer Direct3D 12 | ✅ |
| Audio XACT/DirectSound (dialog, musik, 3D) | ✅ |
| Mouse + keyboard (key mapping, mouse-look, rebind) | ✅ |
| Controller Xbox/PlayStation/Switch via SDL3 + rumble | ✅ |
| Panel setting F10 di dalam game | ✅ |
| Launcher terpisah (grafik/controller/keyboard/folder data, ID & EN) | ✅ |
| MSAA | ❌ belum (renderer ini belum punya multisampled target) |
| 720p widescreen | ❌ masih 480p seperti TV standar |
| System Link (jaringan) | ❌ simbol Winsock/XNet masih trap |

Ini **alpha**. Yang sudah diuji: boot, menu, beberapa menit berjalan, mouse/keyboard aktif, audio terinisialisasi.
Misi panjang belum dituntaskan — anggap masih ada crash di tempat yang belum dijelajahi.

Diukur di mesin pengembangan (Ryzen 5 150 + iGPU): working set **±400 MB**, **±0,6 core** saat di menu, game berjalan
pada **30 FPS** internal (bawaan game, bukan batas port).

---

## 2. Syarat sistem

| | Minimum | Nyaman |
|---|---|---|
| OS | Windows 10 64-bit | Windows 11 64-bit |
| CPU | x86-64 dengan **SSE4.2 + POPCNT** (Intel Nehalem/Core i generasi 1, 2008+; AMD Bulldozer/FX, 2011+), 2 core | 4 thread atau lebih |
| GPU | Direct3D 12 **feature level 11_0**: NVIDIA GTX 600+, AMD HD 7000+, Intel HD 4200 (Haswell)+ | GPU diskrit 2015+ |
| VRAM | 256 MB | 512 MB+ |
| RAM | 2 GB | 4 GB+ |
| Disk | ~7 GB data game + ~0,4 GB hasil build | 12 GB |
| Input | keyboard + mouse | controller XInput/PS/Switch |

Catatan:

- **Core 2 Duo / Phenom II tidak bisa** — CPU-nya hanya SSE4.1, sedangkan build ini dikompilasi `-march=x86-64-v2`.
- GPU dengan FL 11_0 tetapi **tanpa driver D3D12** (mis. Intel Ivy Bridge HD 4000) tidak bisa menjalankan game-nya,
  tetapi **launcher tetap terbuka** (launcher memakai Direct3D 11) untuk mengatur setelan dan folder.

---

## 3. Yang harus kamu punya

- **Disc Xbox NTSC/USA retail** milikmu sendiri — ISO atau hasil ekstrak.
- `default.xbe` dengan **SHA-1 `fe246a74501cd10d2ce89ca5dbda5080d2b2edcc`** (title ID `0x5454009B`, XDK 5849).
  Runtime memeriksa hash ini; kalau beda (PAL/EUR atau versi lain) muncul peringatan
  *"different release… start anyway?"* dan kemungkinan besar crash.
- Versi PS2/PSP **tidak berlaku** untuk port ini.

---

## 4. Instalasi untuk pemain (dari rilis zip)

Rilis berisi alat, bukan game:

```
TheWarriors/
├─ TheWarriorsLauncher.exe
├─ TheWarriors.exe
├─ TheWarriors.dll            <- BARU ADA setelah start pertama
├─ msvcp140.dll, vcruntime140.dll, vcruntime140_1.dll
├─ compiler/                  <- clang + lld + xbe_recomp, dipakai saat start pertama
├─ assets/                    <- taruh ISO (atau hasil ekstrak) milikmu di sini
└─ README.md, THIRD_PARTY.md, LICENSE
```

Langkah:

1. **Ekstrak** zip ke folder yang bisa ditulis (bukan `C:\Program Files`, dan hindari folder yang disinkronkan
   OneDrive). Jangan jalankan dari dalam zip.
2. **Taruh data game** di `assets\`, salah satu bentuk:
   - satu file **`.iso`** Xbox (nama bebas) — paling mudah, tidak perlu ekstrak; atau
   - hasil ekstrak lengkap: `default.xbe`, `Main.wad0..3`, `Paks.wad0..2`, `System.wad0`, `XBoxWad.idx`,
     `audio\`, `bik\`.
   Boleh juga diletakkan di drive lain — lihat langkah 3.
3. **Jalankan `TheWarriorsLauncher.exe`** → tab **Folder game** → arahkan ke folder/ISO-mu (kalau tidak di
   `assets\`) → tombol **MAINKAN**.
4. **Start pertama**: jendela progres muncul, kode game dikompilasi dari XBE milikmu (±1,5–4 menit tergantung CPU,
   129 file). Sekali per rilis; berikutnya langsung masuk.
5. Di dalam game tekan **F10** untuk panel setting (display, sensitivitas, rebind key, controller).

> Jangan menaruh `default.xbe` **sendirian** tanpa file WAD di folder data. Runtime akan memilih folder itu dan game
> mati beberapa ratus milidetik setelah boot dengan `C0000005 write ... guest 00000000`. Ini penyebab crash paling
> menjebak saat bring-up — lihat [PORTING-STATUS.md](PORTING-STATUS.md).

### Launcher

| Tab | Isi |
|---|---|
| **Grafik** | mode layar (fullscreen/borderless/windowed), lebar/tinggi, render scale %, FPS cap, V-Sync, anisotropic filtering, tekstur pengganti |
| **Controller** | kecepatan kamera, deadzone, invert X/Y, getaran + kekuatan, remap tombol pad |
| **Keyboard & Mouse** | aktif/nonaktif, sensitivitas, smoothing, invert Y, klik menu, 25 binding key |
| **Folder game** | folder data (bisa di drive mana pun), executable game, lokasi `settings.toml` |

Bahasa antarmuka: **Indonesia / English** (kanan atas), tersimpan sebagai `launcher_language` di `settings.toml`.
Launcher dan game membaca **file setelan yang sama**, jadi perubahan dari launcher maupun dari panel F10 selalu
konsisten.

---

## 5. Instalasi untuk developer (build dari source)

Butuh:

- **Visual Studio 2022 Build Tools** dengan komponen: *C++ Clang tools for Windows*, *C++ CMake tools for Windows*,
  Windows 10/11 SDK (header & lib D3D12/XAudio2).
- Git, CMake ≥ 3.25, Ninja. Kalau tidak ada di PATH, taruh portable di `..\toolchain\` (git/cmake/ninja) —
  `build-here.bat` menambahkannya sendiri.
- **Disc Xbox NTSC** milikmu sebagai `assets\default.xbe` (+ folder datanya).

Build:

```bat
build-here.bat
```

Alurnya: konfigurasi CMake (mengunduh Zydis/toml++/SDL3/ImGui lewat FetchContent) → `xbe_symbols` memindai XBE dan
menulis `config\game.toml` → `xbe_recomp` menerjemahkan 128 partisi kode game → compile + link.

Hasil di `out\build\release\`:

```
TheWarriors.exe            host: runtime + hook
TheWarriors.dll            kode game hasil recompile (±14,8 MB)
TheWarriorsLauncher.exe    launcher
compiler\                  dipakai untuk build-ulang kode game di PC lain
```

Catatan build:

- Configure **mewajibkan** `assets\default.xbe` ada, karena hash-nya ditanam ke exe. Tanpa dump, repo tidak bisa
  di-build — ini normal untuk proyek recompilation (BYOD).
- `xbe_symbols` menemukan 179/243 fungsi + 10/12 data untuk XBE retail ini; sisanya (Bink, sebagian DirectSound
  lanjutan, Winsock/XNet) diisi manual atau dibiarkan trap. Rincian di [PORTING-STATUS.md](PORTING-STATUS.md).
- Paket rilis: `release.bat` (butuh 7-Zip) atau `package.bat` (tanpa 7-Zip). Keduanya menyusun zip **tanpa**
  `TheWarriors.dll`, **tanpa** data game di `assets\`, dan **tanpa** `compiler\work`.

---

## 6. Kontrol default

| Aksi | Key |
|---|---|
| Jalan | W A S D (Left Alt = walk) |
| Kamera | mouse |
| Serang ringan (X) | Klik kiri |
| Serang berat / charge (A) | Klik kanan, Enter |
| Grab / lempar (B) | E, Backspace |
| Aksi: lompat, panjat, rampok (Y) | Space |
| Sprint (LT) | Left Shift |
| Target / lepas grapple (RT) | Q |
| Block (Black) | F |
| White | R |
| Rage (kedua trigger) | Z |
| Perintah geng (L3) | C |
| Klik stick kanan | Mouse3, V |
| D-pad | Panah, 1 2 3 4 |
| Pause / Back | Esc / Tab |
| Panel setting | F10 |

Semua bisa di-rebind lewat launcher (tab **Keyboard & Mouse**) atau panel F10.

---

## 7. Struktur repositori

```
TheWarriors-XBE/
├─ launcher/main.cpp        launcher: ImGui + D3D11, menulis settings.toml, tombol PLAY
├─ src/
│  ├─ game/                 hook game-side: kbm, camera, prompts, menu, lockpick, movies, dsound, d3d
│  └─ common/               apu_clock, game_info (penyambung hook host)
├─ config/game.toml         alamat simbol + hook untuk XBE ini
├─ sdk/                     XboxXBERecomp (MIT) — recompiler + runtime
├─ build.bat                build lengkap
├─ build-here.bat           build dengan toolchain portabel di ..\toolchain
├─ release.bat              paket rilis (butuh 7-Zip)
├─ package.bat              paket rilis tanpa 7-Zip
├─ Play.bat                 buka launcher (fallback ke game)
├─ PORTING-STATUS.md        catatan bring-up: apa yang ditambal, mengapa, apa yang tersisa
├─ THIRD_PARTY.md           komponen pihak ketiga + lisensi
└─ PANDUAN.md               catatan eksperimen Android (bukan port PC ini)
```

---

## 8. Troubleshooting

| Gejala | Penyebab & solusi |
|---|---|
| Langsung menutup, crash di `MSVCP140.dll` | DLL CRT sistem terlalu lama. Rilis sudah membawa `msvcp140.dll` / `vcruntime140*.dll`; pastikan ada di samping exe |
| `Put the game in the assets folder` | `assets\` kosong, ISO masih ter-zip, atau ekstrak terlalu dalam (`assets\The Warriors\default.xbe`) |
| `...is a different release of the game` | XBE bukan NTSC retail (`fe246a74…`). PAL/EUR butuh build lain |
| Mati ~0,4 detik setelah boot | Folder data tidak lengkap (mis. hanya `default.xbe` tanpa WAD) — pakai ISO atau ekstrak penuh |
| Kompilasi start pertama gagal | Antivirus mengarantina `compiler\clang.exe` / `lld-link.exe`; kembalikan lalu tambahkan exclusion. Detail di `logs\compile.log` |
| Layar hitam / tanpa jendela | Perbarui driver GPU; di laptop dua GPU, set exe ke GPU high-performance |
| Tidak ada suara | Set device playback default Windows sebelum start |
| FPS rendah | Turunkan `render_percent` (60–80), `fps_cap = 30`, `anisotropic_filtering = 0` di launcher |
| Crash di tengah misi | Buka issue dengan `logs\warriors.log` + file terbaru di `CrashDumps\` |

Log `logs\TheWarriors.log`, save `hdd\`, setelan `settings.toml` — semuanya di samping exe.

---

## 9. Lisensi & kredit

Kode asli repositori ini (launcher, skrip build, dokumentasi, perbaikan host-side) dirilis **MIT** — lihat
[LICENSE](LICENSE). Lisensi itu **tidak** memberi hak apa pun atas game, kode game, atau asetnya.

Komponen pihak ketiga dan asal-usul hook game ada di [THIRD_PARTY.md](THIRD_PARTY.md). Yang utama:

- **[XboxXBERecomp](https://github.com/TGP482/XboxXBERecomp)** (TGP482, MIT) — recompiler + runtime.
- **[TheWarriors-XBERecomp](https://github.com/Antriksh1984/TheWarriors-XBERecomp)** (Antriksh1984) — acuan alamat
  simbol, hook DirectSound/D3D/APU, serta lapisan keyboard/mouse dan menu untuk XBE NTSC yang sama.
- XenonRecomp, Zydis, toml++, Dear ImGui, SDL3, LLVM.

*The Warriors* © Rockstar Games / Take-Two Interactive. Proyek ini tidak resmi dan tidak berafiliasi; tidak
menyertakan kode game maupun data game.

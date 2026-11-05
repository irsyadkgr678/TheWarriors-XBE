# Panduan untuk penguji

Dokumen ini ditulis untuk orang yang belum pernah menyentuh proyek ini. Tidak perlu compiler, tidak perlu akun
GitHub, tidak perlu internet setelah unduhan selesai.

## Yang perlu kamu siapkan

1. **Windows 10 atau 11, 64-bit.** Windows 32-bit tidak bisa.
2. **GPU yang mendukung Direct3D 12.** Kurang lebih NVIDIA GTX 600 ke atas, AMD HD 7000 ke atas, atau Intel HD 4200
   (Haswell) ke atas. Cara cepat memeriksa: tekan `Win+R`, tulis `dxdiag`, buka tab **Display**, lihat **Feature
   Levels**. Harus ada `11_0`, `11_1`, `12_0`, atau `12_1`.
3. **Disc Xbox NTSC/USA milikmu sendiri**, dalam bentuk satu file `.iso`. Versi PS2 dan PSP tidak bisa dipakai.
   Ukuran ISO sekitar 5 GB; ini tidak perlu diekstrak.

## Langkah instalasi

1. **Unduh** `TheWarriors-port-v0.1.0-alpha.zip` (sekitar 59 MB).
2. **Ekstrak** ke folder baru yang bisa ditulis, misalnya `D:\Games\TheWarriors`. Jangan di `C:\Program Files`,
   dan jangan jalankan langsung dari dalam zip.
3. **Taruh ISO** di dalam folder `assets`. Hasilnya seperti ini:

   ```
   TheWarriors\
     TheWarriorsLauncher.exe
     TheWarriors.exe
     compiler\
     assets\
       TheWarriors.iso      <- ISO milikmu, nama file bebas
   ```

4. **Jalankan `TheWarriorsLauncher.exe`.**
   Kalau ISO tidak ada di `assets\`, buka tab **Folder game**, pilih folder tempat ISO disimpan, lalu kembali ke
   tab **Grafik** kalau perlu mengatur resolusi.
5. **Tekan MAINKAN.**
   Peluncuran pertama akan mengompilasi kode game dari ISO milikmu. Muncul jendela progres, waktunya sekitar 2–4
   menit tergantung CPU. Ini terjadi sekali; setelah itu game langsung terbuka.
6. Di dalam game, tekan **F10** untuk pengaturan display, sensitivitas mouse, key binding, dan controller.

## Kontrol awal

| Aksi | Key |
|---|---|
| Jalan | W A S D |
| Kamera | mouse |
| Serang ringan / berat | klik kiri / klik kanan |
| Grab / lempar | E atau Backspace |
| Aksi (lompat, panjat) | Space |
| Sprint | Left Shift |
| Pause | Esc |

Semua bisa diubah di launcher atau di panel F10.

## Kalau gagal

| Yang terlihat | Artinya |
|---|---|
| `Put the game in the assets folder` | ISO belum ada, masih di dalam zip, atau letaknya satu folder terlalu dalam |
| `...is a different release of the game` | ISO bukan NTSC/USA retail; build ini tidak mendukung versi lain |
| Jendela tertutup sekitar 0,4 detik setelah dibuka | folder `assets` berisi `default.xbe` lepas tanpa file WAD. Hapus, sisakan ISO saja |
| Kompilasi pertama gagal | antivirus mengarantina `compiler\clang.exe` atau `lld-link.exe`. Kembalikan file itu, lalu tambahkan folder game ke exclusion antivirus |
| Layar hitam | perbarui driver GPU; di laptop dua GPU, paksa `TheWarriors.exe` memakai GPU diskrit |
| Tidak ada suara | set perangkat audio yang diinginkan sebagai default Windows sebelum memulai |

## Kalau mau melaporkan masalah

Kirimkan tiga hal ini:

1. Isi `logs\TheWarriors.log` (kalau crash, ambil 50 baris terakhir).
2. File terbaru di `CrashDumps\`, kalau ada.
3. CPU, GPU, dan RAM.

Jangan lampirkan ISO, file game, atau `TheWarriors.dll` saat melapor.

## Yang perlu diketahui sebelum mencoba

Ini build alpha. Yang sudah berjalan: boot, menu, gameplay, renderer Direct3D 12, audio, mouse dan keyboard,
gamepad. Yang belum ada: MSAA, output 720p, dan System Link. Misi lanjut belum diuji tuntas, jadi crash di tengah
permainan masih mungkin terjadi.

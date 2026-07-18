# Changelog

Semua perubahan penting pada project ini akan didokumentasikan di file ini.

## [1.0.1] - 2026-07-18

### Fixed
- **`library.properties`**: menghapus field `depends=IskakINO_WifiPortal, IskakINO_ArduFast` yang keliru — core library tidak punya dependency kode terhadap keduanya, sehingga tidak seharusnya dipaksa ter-install lewat Arduino Library Manager. Kedua library tersebut kini hanya disebut sebagai opsional di README untuk contoh integrasi.
- **`update()`**: menambahkan validasi ukuran paket UDP (`packetSize >= 48`) sebelum di-parse, mencegah pembacaan byte offset yang tidak valid dari paket yang rusak/terlalu pendek.
- **`isTimeReliable()`**: memperbaiki potensi overflow `uint32_t` saat `maxAgeSeconds * 1000` untuk nilai `maxAgeSeconds` di atas ~4,29 juta detik (~49,7 hari).
- **`setEpoch()`**: sekarang turut mengisi `_bootTimestamp` (konsisten dengan jalur sync NTP otomatis), sehingga `getUptimeSeconds()` tidak lagi diam-diam berpindah jalur perhitungan ketika waktu diset manual tanpa pernah sync NTP.
- **Constructor**: menginisialisasi `_lastUpdateTick` (sebelumnya uninitialized member, meski secara praktik selalu ter-guard oleh `_currentEpoch > 0`).
- **`getDay()`/`getMonth()`/`getYear()`**: menambahkan `#include <time.h>` eksplisit, tidak lagi mengandalkan include transitif dari core ESP8266/ESP32.

### Added
- Overload baru `isAlarmActive(int hr, int min, int sec, bool &firedFlag)` — versi one-shot yang hanya `true` sekali per window waktu, untuk mencegah aksi non-idempoten (toggle relay, notifikasi, counter) tereksekusi berulang kali selama detik yang sama.
- Example baru `01_Basic_Usage.ino` — penggunaan minimal library secara berdiri sendiri, tanpa IskakINO_WifiPortal/ArduFast.

### Changed
- `README.md`: menambahkan dokumentasi untuk `isTimeSet()`, `forceUpdate()`, `setSyncInterval()`, overload `isAlarmActive()` one-shot, catatan tentang semantik `getEpoch()` (epoch lokal, bukan UTC murni), dan tabel daftar examples.
- Example `02`, `06`, `10`: diperbarui untuk menggunakan overload `isAlarmActive()` one-shot agar tidak spam Serial / retrigger berulang.

### Fixed (CI compile error)
- Semua example (`02`–`10`) yang menggunakan `IskakINO_WifiPortal` memakai API yang salah: `portal.begin("Nama")` (1 argumen) dan `portal.handle()`. API asli library tersebut adalah `portal.setBrandName("Nama")` + `portal.begin("Nama-AP", "password")`, dan `portal.tick()` (bukan `handle()`). Semua example diperbaiki untuk memakai API yang benar sehingga lolos compile-check CI (`esp8266:nodemcuv2`).

## [1.0.0] - Initial Release
- Rilis awal: NTP non-blocking, clock simulation, multi-bahasa (ID/EN), fitur uptime, auto-sync.

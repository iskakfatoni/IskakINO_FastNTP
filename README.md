# IskakINO_FastNTP 🚀
<p align="left">
  <img src="https://img.shields.io/github/actions/workflow/status/iskakfatoni/IskakINO_FastNTP/compile_check.yml?branch=main&style=flat-square&label=Build%20Status" alt="Build Status">
  <img src="https://img.shields.io/github/v/release/iskakfatoni/IskakINO_FastNTP?style=flat-square&color=blue" alt="Latest Release">
  <img src="https://img.shields.io/github/license/iskakfatoni/IskakINO_FastNTP?style=flat-square&color=yellow" alt="License">
  <img src="https://img.shields.io/badge/Platform-ESP8266%20%7C%20ESP32-orange?style=flat-square" alt="Platform">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-green?style=flat-square" alt="Language">
</p>

Library NTP (Network Time Protocol) **Non-Blocking** untuk ESP8266 dan ESP32. Dirancang khusus untuk efisiensi tinggi menggunakan *State Machine* sehingga tidak mengganggu proses utama (no delay).

## ✨ Fitur Utama
- **Asinkron**: Tidak membuat program "hang" saat menunggu respon server.
- **Clock Simulation**: Tetap menghitung waktu secara mandiri meski internet terputus.
- **Multi-Language**: Mendukung nama hari dan bulan dalam **Bahasa Indonesia** dan Inggris.
- **Fitur Uptime**: Menghitung berapa lama sistem berjalan berdasarkan waktu nyata.
- **Auto-Sync**: Sinkronisasi ulang otomatis setiap 1 jam (dapat dikustomisasi).
- **Alarm One-Shot**: Overload `isAlarmActive()` yang aman dari retrigger berulang di detik yang sama.

## 📦 Instalasi
Library ini **berdiri sendiri** — cukup `WiFiUdp.h` bawaan platform ESP8266/ESP32, tidak ada dependency wajib lain. Lihat `01_Basic_Usage.ino` untuk contoh minimal.

Beberapa contoh lanjutan (example 02 dst.) mendemonstrasikan integrasi opsional dengan library IskakINO lain — install jika Anda ingin mencoba contoh tersebut:
1. [IskakINO_WifiPortal](https://github.com/iskakfatoni/IskakINO_WifiPortal) — manajemen koneksi WiFi via portal
2. [IskakINO_ArduFast](https://github.com/iskakfatoni/IskakINO_ArduFast) — kontrol pin/relay cepat

## 🚀 Cara Penggunaan Singkat

```cpp
#include <ESP8266WiFi.h> // atau WiFi.h untuk ESP32
#include <WiFiUdp.h>
#include <IskakINO_FastNTP.h>

WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org");

void setup() {
  Serial.begin(115200);
  // GMT+7 (7 * 3600)
  ntp.begin(25200); 
}

void loop() {
  ntp.update(); // Wajib dipanggil di loop tanpa delay

  if (ntp.isTimeReliable()) {
    Serial.println(ntp.getFormattedTime());
    Serial.println(ntp.getDayName(LANG_ID));
  }
}
```

## 🛠 API Reference

Daftar fungsi lengkap yang tersedia di dalam library **IskakINO_FastNTP**:

### 1. Inisialisasi & Core
* `begin(long gmtOffset = 25200, int daylightOffset = 0)` : Inisialisasi NTP dengan offset GMT (detik).
* `update()` : Menjalankan State Machine. Wajib dipanggil di `loop()` tanpa delay.
* `forceUpdate()` : Memaksa sinkronisasi ulang saat itu juga, tanpa menunggu `_syncInterval` habis.
* `setSyncInterval(uint32_t intervalMs)` : Mengubah interval auto-sync (default 3.600.000 ms / 1 jam).
* `setEpoch(uint32_t manualEpoch)` : Mengatur waktu secara manual (misal dari modul RTC eksternal).

### 2. Output Waktu & Tanggal (String)
* `getFormattedTime()` : Output format `HH:MM:SS`.
* `getFormattedDate(char separator)` : Output format `DD-MM-YYYY`.
* `getDayName(NTP_Language lang)` : Nama hari (`LANG_ID` / `LANG_EN`).
* `getMonthName(NTP_Language lang)` : Nama bulan (`LANG_ID` / `LANG_EN`).

### 3. Satuan Waktu (Integer)
* `getEpoch()` : Mengambil nilai epoch internal. ⚠️ Catatan: nilai ini **sudah** ditambah `gmtOffset` + `daylightOffset` (yaitu epoch waktu lokal, bukan Unix epoch UTC murni). Jangan langsung dioper ke sistem lain yang mengharapkan UTC asli tanpa dikurangi offset yang sama.
* `getHours()`, `getMinutes()`, `getSeconds()` : Jam, Menit, Detik.
* `getDay()`, `getMonth()`, `getYear()` : Tanggal, Bulan, Tahun.

### 4. Status & Utilitas
* `isTimeSet()` : Return `true` jika waktu sudah pernah diset sekali (via sync NTP maupun `setEpoch()`).
* `isTimeReliable(uint32_t maxAge = 86400)` : Cek apakah waktu masih "segar" (sinkronisasi/setEpoch terakhir kurang dari `maxAge` detik yang lalu).
* `getUptimeSeconds()` : Durasi sistem berjalan (detik) sejak waktu pertama kali valid.
* `getMillisSinceLastSync()` : Selisih waktu (ms) sejak sinkronisasi/setEpoch terakhir.
* `isAlarmActive(hr, min, sec)` : Return `true` jika waktu sekarang cocok dengan parameter. ⚠️ **Stateless** — akan `true` berulang kali selama detik tersebut berlangsung (bisa ratusan kali per detik di dalam `loop()`). Cocok untuk aksi idempoten seperti `digitalWrite()`.
* `isAlarmActive(hr, min, sec, bool &firedFlag)` : Versi **one-shot** — hanya `true` SEKALI saat kondisi waktu tercapai, lalu otomatis reset saat waktunya sudah lewat. Gunakan `static bool` milik Anda sendiri sebagai `firedFlag` per-alarm. Cocok untuk aksi non-idempoten seperti toggle relay, kirim notifikasi, atau increment counter.

  ```cpp
  static bool alarmFired = false;
  if (ntp.isAlarmActive(18, 0, 0, alarmFired)) {
    // Dieksekusi TEPAT SEKALI saat jam 18:00:00 tiba
  }
  ```

## 📂 Daftar Contoh (Examples)

| File | Deskripsi |
| :--- | :--- |
| `01_Basic_Usage` | Penggunaan paling dasar, berdiri sendiri tanpa library IskakINO lain. |
| `02_Smart_Alarm_ArduFast` | Alarm relay dengan `isAlarmActive()` one-shot + WifiPortal + ArduFast. |
| `03_Advanced_Scheduler_ArduFast` | Indikator status via ArduFast + laporan uptime/reliabilitas. |
| `04_Manual_Set_Time` | Injeksi waktu manual via `setEpoch()` (mis. dari RTC). |
| `05_Calendar_Localization` | Format kalender Indonesia vs Inggris. |
| `06_Weekly_Scheduler_ArduFast` | Penjadwalan mingguan (Senin–Jumat) dengan alarm one-shot. |
| `07_Dynamic_Sync_Interval` | Mengubah interval sync secara dinamis (siang/malam). |
| `08_NTP_Status_LED_ArduFast` | Indikator LED multi-kondisi (WiFi/NTP status). |
| `09_System_Health_Monitor` | Laporan kesehatan sistem: uptime, last sync, drift alert. |
| `10_Full_Integrated_Project` | Proyek smart home lengkap menggabungkan semua fitur. |

## 👥 Kontributor

Kami sangat menghargai kontribusi dalam bentuk pelaporan bug, saran fitur, maupun *pull request*.

| Nama | Peran | GitHub |
| :--- | :--- | :--- |
| **iskakfatoni** | Developer Utama | [@iskakfatoni](https://github.com/iskakfatoni) |

---
*Dibuat oleh [iskakfatoni](https://github.com/iskakfatoni) untuk komunitas IoT Indonesia.*

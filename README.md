# IskakINO_FastNTP 🚀
<p align="left">
  <img src="https://img.shields.io/github/actions/workflow/status/iskakfatoni/IskakINO_FastNTP/compile_check.yml?branch=main&style=flat-square&label=Build%20Status" alt="Build Status">
  <img src="https://img.shields.io/github/v/release/iskakfatoni/IskakINO_FastNTP?style=flat-square&color=blue" alt="Latest Release">
  <img src="https://img.shields.io/github/license/iskakfatoni/IskakINO_FastNTP?style=flat-square&color=yellow" alt="License">
  <img src="https://img.shields.io/badge/Platform-ESP8266%20%7C%20ESP32-orange?style=flat-square" alt="Platform">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-green?style=flat-square" alt="Language">
</p>
[![IskakINO Library CI](https://github.com/iskakfatoni/IskakINO_FastNTP/actions/workflows/compile_check.yml/badge.svg)](https://github.com/iskakfatoni/IskakINO_FastNTP/actions/workflows/compile_check.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Library NTP (Network Time Protocol) **Non-Blocking** untuk ESP8266 dan ESP32. Dirancang khusus untuk efisiensi tinggi menggunakan *State Machine* sehingga tidak mengganggu proses utama (no delay).

## ✨ Fitur Utama
- **Asinkron**: Tidak membuat program "hang" saat menunggu respon server.
- **Clock Simulation**: Tetap menghitung waktu secara mandiri meski internet terputus.
- **Multi-Language**: Mendukung nama hari dan bulan dalam **Bahasa Indonesia** dan Inggris.
- **Fitur Uptime**: Menghitung berapa lama sistem berjalan berdasarkan waktu nyata.
- **Auto-Sync**: Sinkronisasi ulang otomatis setiap 1 jam (dapat dikustomisasi).

## 📦 Instalasi
Library ini memerlukan library pendukung berikut:
1. [IskakINO_WifiPortal](https://github.com/iskakfatoni/IskakINO_WifiPortal)
2. [IskakINO_ArduFast](https://github.com/iskakfatoni/IskakINO_ArduFast)

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
* `begin(long gmtOffset, int daylightOffset)` : Inisialisasi NTP dengan offset GMT (detik).
* `update()` : Menjalankan State Machine. Wajib dipanggil di `loop()` tanpa delay.
* `setEpoch(uint32_t manualEpoch)` : Mengatur waktu secara manual.

### 2. Output Waktu & Tanggal (String)
* `getFormattedTime()` : Output format `HH:MM:SS`.
* `getFormattedDate(char separator)` : Output format `DD-MM-YYYY`.
* `getDayName(NTP_Language lang)` : Nama hari (`LANG_ID` / `LANG_EN`).
* `getMonthName(NTP_Language lang)` : Nama bulan (`LANG_ID` / `LANG_EN`).

### 3. Satuan Waktu (Integer)
* `getEpoch()` : Mengambil nilai Unix Epoch.
* `getHours()`, `getMinutes()`, `getSeconds()` : Jam, Menit, Detik.
* `getDay()`, `getMonth()`, `getYear()` : Tanggal, Bulan, Tahun.

### 4. Status & Utilitas
* `isTimeReliable(uint32_t maxAge)` : Cek validitas waktu (default maxAge 24 jam).
* `getUptimeSeconds()` : Durasi sistem berjalan (detik) sejak boot.
* `getMillisSinceLastSync()` : Selisih waktu sejak sinkronisasi sukses terakhir.
* `isAlarmActive(hr, min, sec)` : Return `true` jika waktu sekarang cocok dengan parameter.

## 👥 Kontributor

Kami sangat menghargai kontribusi dalam bentuk pelaporan bug, saran fitur, maupun *pull request*.

| Nama | Peran | GitHub |
| :--- | :--- | :--- |
| **iskakfatoni** | Developer Utama | [@iskakfatoni](https://github.com/iskakfatoni) |

---
*Dibuat oleh [iskakfatoni](https://github.com/iskakfatoni) untuk komunitas IoT Indonesia.*


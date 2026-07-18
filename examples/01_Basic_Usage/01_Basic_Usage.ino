/*
 * Library: IskakINO_FastNTP
 * Example: 01_Basic_Usage
 * ------------------------------------------------------------
 * Deskripsi:
 * Contoh PALING DASAR penggunaan IskakINO_FastNTP secara berdiri
 * sendiri (standalone), TANPA IskakINO_WifiPortal maupun
 * IskakINO_ArduFast. Hanya butuh koneksi WiFi biasa.
 *
 * Cocok untuk:
 * - Memahami alur begin() -> update() -> getFormattedTime()
 * - Testing cepat sebelum diintegrasikan ke project yang lebih besar
 * ------------------------------------------------------------
 */

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <IskakINO_FastNTP.h>

// --- Ganti dengan kredensial WiFi Anda ---
const char* WIFI_SSID = "NAMA_WIFI_ANDA";
const char* WIFI_PASS = "PASSWORD_WIFI_ANDA";

WiFiUDP ntpUDP;
// Server NTP Indonesia, bisa diganti "pool.ntp.org" untuk server global
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org");

unsigned long prevMs = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("--- Example 01: Basic Usage ---"));

  // 1. Sambungkan WiFi seperti biasa
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print(F("Menyambungkan ke WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.print(F("WiFi Terhubung! IP: "));
  Serial.println(WiFi.localIP());

  // 2. Mulai NTP. GMT+7 (WIB) = 7 * 3600 = 25200 detik.
  //    Untuk WITA (GMT+8) gunakan 28800, WIT (GMT+9) gunakan 32400.
  ntp.begin(25200);

  Serial.println(F("Menunggu sinkronisasi waktu pertama..."));
}

void loop() {
  // 3. WAJIB dipanggil terus-menerus di loop(), tanpa delay().
  //    Library ini non-blocking: update() hanya memproses state machine
  //    secukupnya lalu langsung return.
  ntp.update();

  // 4. Cetak waktu setiap 1 detik, hanya jika waktu sudah pernah diset
  if (millis() - prevMs >= 1000) {
    prevMs = millis();

    if (ntp.isTimeSet()) {
      Serial.print(F("[")); 
      Serial.print(ntp.getDayName(LANG_ID));
      Serial.print(F(", "));
      Serial.print(ntp.getFormattedDate('-'));
      Serial.print(F("] "));
      Serial.println(ntp.getFormattedTime());

      if (!ntp.isTimeReliable()) {
        Serial.println(F("  (Peringatan: sudah lebih dari 24 jam sejak sync terakhir)"));
      }
    } else {
      Serial.println(F("Menunggu sinkronisasi NTP..."));
    }
  }
}

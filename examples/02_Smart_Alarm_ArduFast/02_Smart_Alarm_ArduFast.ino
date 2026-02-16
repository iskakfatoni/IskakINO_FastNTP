/*
 * Library: IskakINO_FastNTP
 * Example: 02_Smart_Alarm_ArduFast
 * ------------------------------------------------------------
 * Deskripsi:
 * Menggabungkan 3 Library IskakINO:
 * 1. WifiPortal -> Manajemen Koneksi WiFi.
 * 2. ArduFast   -> Kontrol Pin/Relay yang cepat & efisien.
 * 3. FastNTP    -> Penjadwalan waktu presisi (Non-blocking).
 * ------------------------------------------------------------
 */

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <IskakINO_WifiPortal.h>  
#include <IskakINO_FastNTP.h>     
#include <IskakINO_ArduFast.h>    // Library I/O Cepat Anda

// Pin Relay (Contoh menggunakan pin D1/GPIO5)
#define RELAY_PIN 5

// Inisialisasi Objek
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org"); 
IskakINO_WifiPortal portal;
IskakINO_ArduFast io; // Objek untuk kontrol pin cepat

unsigned long prevMs = 0;

void setup() {
  Serial.begin(115200);
  
  // 1. Setup Pin via ArduFast (Set sebagai output)
  io.pinMode(RELAY_PIN, OUTPUT);
  io.digitalWrite(RELAY_PIN, LOW); // Matikan relay saat awal

  // 2. Mulai Portal
  portal.begin("IskakINO_SmartHome");

  // 3. Mulai NTP (WIB = GMT+7)
  ntp.begin(25200); 

  Serial.println(F("Smart Alarm System Ready..."));
}

void loop() {
  // Selalu jalankan handle portal
  portal.handle();

  // Update State Machine NTP jika WiFi OK
  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // LOGIKA ALARM: Menggunakan isAlarmActive()
  // Contoh: Nyalakan lampu setiap jam 18:00:00
  if (ntp.isAlarmActive(18, 0, 0)) {
    io.digitalWrite(RELAY_PIN, HIGH);
    Serial.println(F("ALARM: Lampu Dinyalakan!"));
  }

  // Contoh: Matikan lampu setiap jam 06:00:00
  if (ntp.isAlarmActive(6, 0, 0)) {
    io.digitalWrite(RELAY_PIN, LOW);
    Serial.println(F("ALARM: Lampu Dimatikan!"));
  }

  // Tampilkan Status ke Serial setiap 1 detik
  if (millis() - prevMs >= 1000) {
    prevMs = millis();
    if (ntp.isTimeSet()) {
      Serial.print(F("[TIME] "));
      Serial.print(ntp.getDayName(LANG_ID));
      Serial.print(F(", "));
      Serial.println(ntp.getFormattedTime());
    }
  }
}

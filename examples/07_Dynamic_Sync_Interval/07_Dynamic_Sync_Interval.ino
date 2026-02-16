/*
 * Library: IskakINO_FastNTP
 * Example: 07_Dynamic_Sync_Interval
 * ------------------------------------------------------------
 * Deskripsi:
 * Menggabungkan IskakINO_WifiPortal, IskakINO-ArduFast, dan IskakINO_FastNTP.
 * Fitur: Mengubah interval sinkronisasi secara dinamis berdasarkan waktu.
 * Siang (06:00 - 18:00): Sinkron tiap 30 menit.
 * Malam (18:01 - 05:59): Sinkron tiap 6 jam.
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
#include <IskakINO_ArduFast.h>    

WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org"); 
IskakINO_WifiPortal portal;
IskakINO_ArduFast io;

void setup() {
  Serial.begin(115200);
  
  portal.begin("IskakINO_Dynamic");
  ntp.begin(25200); // GMT+7

  Serial.println(F("--- Example 07: Dynamic Sync ---"));
}

void loop() {
  portal.handle();

  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // --- LOGIKA DYNAMIC INTERVAL ---
  static int lastHourCheck = -1;
  int currentHour = ntp.getHours();

  // Cek setiap kali jam berubah
  if (currentHour != lastHourCheck && ntp.isTimeSet()) {
    lastHourCheck = currentHour;

    // Jika jam 06:00 sampai 18:00 (Siang)
    if (currentHour >= 6 && currentHour < 18) {
      ntp.setSyncInterval(1800000); // 30 Menit (ms)
      Serial.println(F("[MODE] Siang: Sinkronisasi dipercepat (30 menit)"));
    } 
    // Jika Malam
    else {
      ntp.setSyncInterval(21600000); // 6 Jam (ms)
      Serial.println(F("[MODE] Malam: Sinkronisasi diperlambat (6 jam)"));
    }
  }

  // Monitor status sinkronisasi
  static unsigned long prevMs = 0;
  if (millis() - prevMs >= 10000) { // Cek tiap 10 detik
    prevMs = millis();
    if (ntp.isTimeSet()) {
      Serial.print(F("Waktu: ")); Serial.print(ntp.getFormattedTime());
      Serial.print(F(" | Ms sejak sync terakhir: ")); 
      Serial.println(ntp.getMillisSinceLastSync());
    }
  }
}

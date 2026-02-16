/*
 * Library: IskakINO_FastNTP
 * Example: 05_Calendar_Localization
 * ------------------------------------------------------------
 * Deskripsi:
 * Contoh penggunaan fitur penamaan bulan dan format tanggal.
 * Menggabungkan IskakINO_WifiPortal, IskakINO-ArduFast, dan IskakINO_FastNTP.
 * Mendukung Bahasa Indonesia (LANG_ID) dan Inggris (LANG_EN).
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

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  
  // Portal untuk koneksi WiFi
  portal.begin("IskakINO_Calendar");

  // Inisialisasi NTP (GMT+7 untuk Indonesia)
  ntp.begin(25200); 

  Serial.println(F("--- Demo Lokalisasi Kalender ---"));
}

void loop() {
  portal.handle();

  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // Cetak format kalender lengkap setiap 5 detik
  if (millis() - lastPrint >= 5000) {
    lastPrint = millis();

    if (ntp.isTimeSet()) {
      Serial.println(F("\n>>> FORMAT KALENDER INDONESIA <<<"));
      // Output: Senin, 16 Februari 2026
      Serial.print(ntp.getDayName(LANG_ID));
      Serial.print(F(", "));
      Serial.print(ntp.getDay());
      Serial.print(F(" "));
      Serial.print(ntp.getMonthName(LANG_ID));
      Serial.print(F(" "));
      Serial.println(ntp.getYear());
      
      // Output Format Tanggal (Custom Separator)
      Serial.print(F("Short Date: "));
      Serial.println(ntp.getFormattedDate('/')); // 16/02/2026

      Serial.println(F("\n>>> ENGLISH CALENDAR FORMAT <<<"));
      // Output: Monday, February 16 2026
      Serial.print(ntp.getDayName(LANG_EN));
      Serial.print(F(", "));
      Serial.print(ntp.getMonthName(LANG_EN));
      Serial.print(F(" "));
      Serial.print(ntp.getDay());
      Serial.print(F(" "));
      Serial.println(ntp.getYear());

      Serial.println(F("--------------------------------"));
    } else {
      Serial.println(F("Menunggu sinkronisasi waktu..."));
    }
  }
}

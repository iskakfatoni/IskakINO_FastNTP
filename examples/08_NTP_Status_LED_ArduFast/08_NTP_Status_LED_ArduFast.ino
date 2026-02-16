/*
 * Library: IskakINO_FastNTP
 * Example: 08_NTP_Status_LED_ArduFast
 * ------------------------------------------------------------
 * Deskripsi:
 * Menggunakan IskakINO-ArduFast untuk membuat indikator status LED:
 * 1. Kedip Cepat (100ms)  : Sedang menyambungkan WiFi (Portal Active).
 * 2. Kedip Lambat (1000ms): WiFi Terhubung, tapi belum Sinkron NTP.
 * 3. LED Panteng (ON)    : WiFi Terhubung & Waktu Sudah Sinkron.
 * 4. LED Mati (OFF)      : WiFi Terputus.
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

// Gunakan ArduFast untuk mendefinisikan Pin LED (Contoh Pin 2)
#define STATUS_LED 2

WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org"); 
IskakINO_WifiPortal portal;
IskakINO_ArduFast io;

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Pin via ArduFast
  io.pinMode(STATUS_LED, OUTPUT);

  portal.begin("IskakINO_Indikator");
  ntp.begin(25200);
}

void loop() {
  portal.handle();
  
  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // --- LOGIKA INDIKATOR STATUS ---
  static unsigned long lastBlink = 0;
  unsigned long currentMs = millis();

  // KONDISI 1: WiFi Belum Terhubung (Portal sedang menunggu config)
  if (WiFi.status() != WL_CONNECTED) {
    if (currentMs - lastBlink >= 100) { // Kedip sangat cepat
      lastBlink = currentMs;
      io.digitalWrite(STATUS_LED, !io.digitalRead(STATUS_LED));
    }
  } 
  // KONDISI 2: WiFi OK, Tapi NTP Belum Sinkron
  else if (!ntp.isTimeSet()) {
    if (currentMs - lastBlink >= 1000) { // Kedip lambat
      lastBlink = currentMs;
      io.digitalWrite(STATUS_LED, !io.digitalRead(STATUS_LED));
    }
  } 
  // KONDISI 3: Semua OK (WiFi Terhubung & NTP Sinkron)
  else {
    io.digitalWrite(STATUS_LED, HIGH); // LED Menyala terus
  }

  // Log Waktu ke Serial setiap 2 detik
  static unsigned long lastLog = 0;
  if (currentMs - lastLog >= 2000) {
    lastLog = currentMs;
    if (ntp.isTimeSet()) {
      Serial.println("System Running: " + ntp.getFormattedTime());
    }
  }
}

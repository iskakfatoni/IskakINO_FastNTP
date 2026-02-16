/*
 * Library: IskakINO_FastNTP
 * Example: 10_Full_Integrated_Project (SMART AUTOMATION)
 * ------------------------------------------------------------
 * Deskripsi:
 * Implementasi lengkap Smart Home System:
 * 1. IskakINO_WifiPortal  : Manajemen koneksi & Web Configuration.
 * 2. IskakINO-ArduFast    : Kontrol Relay Lampu & Indikator LED.
 * 3. IskakINO_FastNTP     : Penjadwalan, Hari Indonesia, & Uptime.
 * * Skenario:
 * - Lampu Teras (Relay) ON jam 18:00, OFF jam 06:00.
 * - LED Indikator status sistem (ArduFast).
 * - Monitor kesehatan sistem setiap 1 menit.
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

// Konfigurasi Pin dengan ArduFast
#define PIN_RELAY_TERAS 5  // Relay untuk Lampu
#define PIN_LED_STATUS  2  // Indikator Sistem

// Inisialisasi Objek
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org"); 
IskakINO_WifiPortal portal;
IskakINO_ArduFast io;

void setup() {
  Serial.begin(115200);
  
  // 1. Setup I/O via ArduFast
  io.pinMode(PIN_RELAY_TERAS, OUTPUT);
  io.pinMode(PIN_LED_STATUS, OUTPUT);
  io.digitalWrite(PIN_RELAY_TERAS, LOW);

  // 2. Mulai Portal WiFi
  portal.begin("IskakINO_SmartHome");

  // 3. Mulai NTP (GMT+7)
  ntp.begin(25200);
  
  Serial.println(F("======================================"));
  Serial.println(F("   ISKAKINO SMART HOME SYSTEM READY   "));
  Serial.println(F("======================================"));
}

void loop() {
  // --- LAYER 1: MAINTENANCE (Wajib) ---
  portal.handle();
  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // --- LAYER 2: VISUAL INDICATOR (ArduFast) ---
  static unsigned long blinkMs = 0;
  if (!ntp.isTimeSet()) {
    if (millis() - blinkMs >= 500) {
      blinkMs = millis();
      io.digitalWrite(PIN_LED_STATUS, !io.digitalRead(PIN_LED_STATUS));
    }
  } else {
    io.digitalWrite(PIN_LED_STATUS, HIGH); // On terus jika sudah sinkron
  }

  // --- LAYER 3: AUTOMATION LOGIC (FastNTP) ---
  // Aksi Jam 18:00:00 (Nyalakan Lampu Teras)
  if (ntp.isAlarmActive(18, 0, 0)) {
    io.digitalWrite(PIN_RELAY_TERAS, HIGH);
    Serial.println(F("[EVENT] Jam 18:00: Lampu Teras ON"));
  }

  // Aksi Jam 06:00:00 (Matikan Lampu Teras)
  if (ntp.isAlarmActive(6, 0, 0)) {
    io.digitalWrite(PIN_RELAY_TERAS, LOW);
    Serial.println(F("[EVENT] Jam 06:00: Lampu Teras OFF"));
  }

  // --- LAYER 4: SERIAL TELEMETRY ---
  static unsigned long reportMs = 0;
  if (millis() - reportMs >= 60000) { // Laporan per menit
    reportMs = millis();
    if (ntp.isTimeSet()) {
      Serial.print(F("[INFO] "));
      Serial.print(ntp.getDayName(LANG_ID));
      Serial.print(F(", "));
      Serial.print(ntp.getFormattedDate('/'));
      Serial.print(F(" | Jam: "));
      Serial.print(ntp.getFormattedTime());
      Serial.print(F(" | Uptime: "));
      Serial.print(ntp.getUptimeSeconds() / 60);
      Serial.println(F(" Menit"));
    }
  }
}

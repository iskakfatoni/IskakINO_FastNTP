/*
 * Library: IskakINO_FastNTP
 * Example: 03_Advanced_Scheduler_ArduFast
 * ------------------------------------------------------------
 * Deskripsi:
 * Mengintegrasikan IskakINO_WifiPortal, IskakINO-ArduFast, dan IskakINO_FastNTP.
 * Menggunakan ArduFast untuk efisiensi I/O dan FastNTP untuk logika uptime
 * serta pengecekan reliabilitas waktu sebelum mengeksekusi tugas.
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

// Konfigurasi Pin menggunakan ArduFast
#define LED_INDICATOR 2  // LED Builtin biasanya pin 2 (ESP32) atau 16/2 (ESP8266)

WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org"); 
IskakINO_WifiPortal portal;
IskakINO_ArduFast io;

unsigned long prevMs = 0;

void setup() {
  Serial.begin(115200);
  
  // ArduFast: Setup pin indikator
  io.pinMode(LED_INDICATOR, OUTPUT);
  io.digitalWrite(LED_INDICATOR, HIGH); // Matikan (Active Low)

  // Memulai Portal WiFi
  portal.begin("IskakINO_Project");

  // Memulai NTP (WIB)
  ntp.begin(25200); 
  
  Serial.println(F("System 03: Advanced Scheduler Ready"));
}

void loop() {
  portal.handle();

  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // Efisiensi ArduFast: Indikator LED berkedip jika waktu belum sinkron
  if (!ntp.isTimeSet()) {
    static unsigned long flashMs = 0;
    if (millis() - flashMs >= 200) {
      flashMs = millis();
      // Toggle pin secara cepat dengan ArduFast
      io.digitalWrite(LED_INDICATOR, !io.digitalRead(LED_INDICATOR));
    }
  } else {
    io.digitalWrite(LED_INDICATOR, HIGH); // Tetap matikan jika sudah sinkron
  }

  // Tampilan Serial Monitor per detik
  if (millis() - prevMs >= 1000) {
    prevMs = millis();

    if (ntp.isTimeSet()) {
      Serial.println("========================================");
      Serial.print(F("Waktu: ")); Serial.println(ntp.getFormattedTime());
      Serial.print(F("Tanggal: ")); Serial.println(ntp.getFormattedDate('/'));
      Serial.print(F("Hari: ")); Serial.println(ntp.getDayName(LANG_ID));
      
      // Fitur Uptime: Menghitung berapa lama alat sudah sinkron & berjalan
      Serial.print(F("Uptime Alat: ")); 
      Serial.print(ntp.getUptimeSeconds()); 
      Serial.println(F(" detik"));

      // Cek Reliabilitas: Apakah sinkronisasi masih segar (kurang dari 1 jam)?
      if (ntp.isTimeReliable(3600)) {
        Serial.println(F("Status: Waktu sangat akurat (Sync OK)"));
      } else {
        Serial.println(F("Status: Perlu sinkronisasi ulang..."));
      }
      Serial.println("========================================");
    }
  }

  // Contoh Logika Scheduler: Aktifkan sesuatu setiap detik ke-30
  if (ntp.getSeconds() == 30) {
     // Lakukan tugas cepat di sini...
  }
}

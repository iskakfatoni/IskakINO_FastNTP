/*
 * Library: IskakINO_FastNTP
 * Example: 06_Weekly_Scheduler_ArduFast
 * ------------------------------------------------------------
 * Deskripsi:
 * Menggabungkan IskakINO_WifiPortal, IskakINO-ArduFast, dan IskakINO_FastNTP.
 * Logika Penjadwalan Mingguan: Menjalankan aksi hanya pada hari-hari tertentu.
 * Contoh: Relay hanya aktif di hari Senin - Jumat jam 08:00:00.
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

// Konfigurasi Pin Relay menggunakan ArduFast
#define PIN_RELAY_KANTOR 5

WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "id.pool.ntp.org"); 
IskakINO_WifiPortal portal;
IskakINO_ArduFast io;

void setup() {
  Serial.begin(115200);
  
  // ArduFast Setup
  io.pinMode(PIN_RELAY_KANTOR, OUTPUT);
  io.digitalWrite(PIN_RELAY_KANTOR, LOW); 

  // Inisialisasi Portal & NTP
  portal.begin("IskakINO_Office");
  ntp.begin(25200); // WIB
}

void loop() {
  portal.handle();

  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // --- LOGIKA PENJADWALAN MINGGUAN ---
  
  // 1. Ambil Nama Hari saat ini
  String hariIni = ntp.getDayName(LANG_EN); // Gunakan English agar lebih stabil untuk pembanding string

  // 2. Logika: Senin s/d Jumat jam 08:00:00 (Waktu Masuk Kantor)
  if (hariIni != "Saturday" && hariIni != "Sunday") {
    
    // Cek jam masuk (08:00:00)
    if (ntp.isAlarmActive(8, 0, 0)) {
      io.digitalWrite(PIN_RELAY_KANTOR, HIGH);
      Serial.println(F("KERJA: Nyalakan fasilitas kantor!"));
    }

    // Cek jam pulang (17:00:00)
    if (ntp.isAlarmActive(17, 0, 0)) {
      io.digitalWrite(PIN_RELAY_KANTOR, LOW);
      Serial.println(F("PULANG: Matikan fasilitas kantor!"));
    }
  }

  // Tampilan Serial Monitor per menit (agar tidak spam)
  static int lastMin = -1;
  if (ntp.getMinutes() != lastMin) {
    lastMin = ntp.getMinutes();
    if (ntp.isTimeSet()) {
      Serial.print(F("Status Sekarang: "));
      Serial.print(ntp.getDayName(LANG_ID));
      Serial.print(F(", "));
      Serial.println(ntp.getFormattedTime());
    }
  }
}

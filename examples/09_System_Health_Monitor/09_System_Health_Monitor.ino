/*
 * Library: IskakINO_FastNTP
 * Example: 09_System_Health_Monitor
 * ------------------------------------------------------------
 * Deskripsi:
 * Menggunakan IskakINO_FastNTP untuk memantau "kesehatan" sistem:
 * 1. Uptime Tracking: Sudah berapa lama alat berjalan sejak sinkron pertama.
 * 2. Sync Status: Kapan terakhir kali sukses mengambil waktu dari internet.
 * 3. Drift Alert: Memberi peringatan jika waktu sudah tidak sinkron lebih dari 24 jam.
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

unsigned long reportTimer = 0;

void setup() {
  Serial.begin(115200);
  
  portal.begin("IskakINO_Health");
  ntp.begin(25200); // GMT+7
  
  Serial.println(F("--- Example 09: System Health Monitor ---"));
}

void loop() {
  portal.handle();
  
  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }

  // Cetak Laporan Kesehatan Sistem setiap 30 detik
  if (millis() - reportTimer >= 30000) {
    reportTimer = millis();
    
    if (ntp.isTimeSet()) {
      Serial.println(F("\n========== SYSTEM HEALTH REPORT =========="));
      
      // 1. Cek Waktu Sekarang
      Serial.print(F("Current Time  : ")); Serial.println(ntp.getFormattedTime());
      
      // 2. Cek Uptime (Sejak sinkron pertama sukses)
      uint32_t up = ntp.getUptimeSeconds();
      int hari = up / 86400;
      int jam  = (up % 86400) / 3600;
      int menit = (up % 3600) / 60;
      
      Serial.print(F("System Uptime : "));
      Serial.print(hari); Serial.print(F("d "));
      Serial.print(jam);  Serial.print(F("h "));
      Serial.print(menit); Serial.println(F("m"));
      
      // 3. Cek Status Sinkronisasi Terakhir
      uint32_t lastSync = ntp.getMillisSinceLastSync() / 1000;
      Serial.print(F("Last Sync     : ")); 
      Serial.print(lastSync); Serial.println(F(" seconds ago"));
      
      // 4. Analisis Reliabilitas (Batas 24 jam = 86400 detik)
      if (ntp.isTimeReliable(86400)) {
        Serial.println(F("Time Status   : [ HEALTHY ]"));
      } else {
        Serial.println(F("Time Status   : [ DRIFT WARNING - Need Sync! ]"));
      }
      
      Serial.println(F("=========================================="));
    } else {
      Serial.println(F("System Status: Waiting for first network sync..."));
    }
  }
}

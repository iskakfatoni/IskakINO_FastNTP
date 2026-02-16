/*
 * Library: IskakINO_FastNTP
 * Example: 04_Manual_Set_Time
 * ------------------------------------------------------------
 * Deskripsi:
 * Menggunakan fitur setEpoch() untuk mengatur waktu secara manual.
 * Sangat berguna jika:
 * 1. Internet mati total tapi Anda punya modul RTC (DS3231).
 * 2. User menginput waktu manual melalui IskakINO_WifiPortal.
 * 3. Tetap menggunakan efisiensi IskakINO-ArduFast untuk kontrol pin.
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
IskakINO_FastNTP ntp(ntpUDP); 
IskakINO_WifiPortal portal;
IskakINO_ArduFast io;

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Portal (Tanpa nunggu koneksi untuk contoh ini)
  portal.begin("IskakINO_Manual_Clock");

  // Inisialisasi NTP seperti biasa
  ntp.begin(25200); 

  Serial.println(F("--- Mode Manual Set Time ---"));
  
  // SIMULASI: Seolah-olah kita membaca data dari RTC atau Input User
  // Contoh Epoch: 1739712000 (Senin, 16 Feb 2026 13:00:00 WIB)
  uint32_t manualTime = 1739712000; 
  
  Serial.println(F("Menyuntikkan waktu manual ke library..."));
  ntp.setEpoch(manualTime); 
  
  Serial.println(F("Waktu berhasil diset manual!"));
}

void loop() {
  // Portal tetap berjalan untuk manajemen WiFi
  portal.handle();

  // Update tetap dijalankan. 
  // Jika WiFi terhubung, dia akan sinkron otomatis ke internet.
  // Jika WiFi mati, library akan melanjutkan hitungan dari waktu manual tadi.
  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  } else {
    // Simulasi jam internal tetap berjalan meskipun offline
    ntp.update(); 
  }

  // Tampilkan hasil setiap detik
  static unsigned long prevMs = 0;
  if (millis() - prevMs >= 1000) {
    prevMs = millis();
    
    Serial.print(F("[STATUS: "));
    Serial.print(WiFi.status() == WL_CONNECTED ? "ONLINE" : "OFFLINE");
    Serial.print(F("] "));
    
    Serial.print(ntp.getDayName(LANG_ID));
    Serial.print(F(", "));
    Serial.println(ntp.getFormattedTime());
    
    if (!ntp.isTimeReliable(60)) {
       Serial.println(F("Peringatan: Waktu berjalan dalam mode OFFLINE (Belum sync server)"));
    }
  }
}

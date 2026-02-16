#ifndef ISKAKINO_FASTNTP_H
#define ISKAKINO_FASTNTP_H

#include <Arduino.h>
#include <Udp.h> 

/**
 * @enum NTP_Language
 * Pilihan bahasa untuk nama hari dan bulan.
 */
enum NTP_Language { LANG_EN, LANG_ID };

/**
 * @enum NTP_State
 * Status State Machine untuk proses non-blocking.
 */
enum NTP_State { STATE_IDLE, STATE_SEND_REQUEST, STATE_AWAIT_RESPONSE };

class IskakINO_FastNTP {
  private:
    UDP* _udp;                     
    const char* _ntpServer;        
    long _gmtOffsetSec;            
    int _daylightOffsetSec;        
    
    uint32_t _syncInterval;        
    uint32_t _lastSyncMs;          
    uint32_t _requestMs;           
    uint32_t _currentEpoch;        
    uint32_t _lastUpdateTick;      
    
    NTP_State _state = STATE_IDLE; 
    byte _packetBuffer[48];        

    void sendNTPPacket();          

  public:
    IskakINO_FastNTP(UDP& udp, const char* server = "pool.ntp.org");

    void begin(long gmtOffset = 25200, int daylightOffset = 0);
    void update(); 
    void forceUpdate();
    
    // --- Status & Validasi ---
    bool isTimeSet() const { return _currentEpoch > 0; }
    bool isTimeReliable(uint32_t maxAgeSeconds = 86400); // Cek jika sinkron terakhir < 24 jam
    uint32_t getEpoch();
    uint32_t getMillisSinceLastSync();

    // --- Getters Waktu (Satuan) ---
    int getSeconds();
    int getMinutes();
    int getHours();
    int getDay();
    int getMonth();
    int getYear();
    
    // --- Getters Nama (Multibahasa) ---
    String getDayName(NTP_Language lang = LANG_ID);
    String getMonthName(NTP_Language lang = LANG_ID);
    
    // --- Getters Formatted String ---
    String getFormattedTime();                       // HH:MM:SS
    String getFormattedDate(char separator = '-');   // DD-MM-YYYY

    // --- Pengaturan ---
    void setSyncInterval(uint32_t intervalMs) { _syncInterval = intervalMs; }
    void setEpoch(uint32_t manualEpoch);             // Manual override jika diperlukan
};

#endif

#ifndef ISKAKINO_FASTNTP_H
#define ISKAKINO_FASTNTP_H

#include <Arduino.h>
#include <Udp.h>
#include <time.h>   // FIX: eksplisit di-include, jangan mengandalkan include transitif dari core ESP8266/ESP32

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
    uint32_t _bootTimestamp;       // Menyimpan epoch saat pertama kali waktu valid (sync NTP ATAU setEpoch manual)

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
    bool isTimeReliable(uint32_t maxAgeSeconds = 86400);
    uint32_t getEpoch();
    uint32_t getUptimeSeconds();        // Total waktu berjalan sejak waktu pertama kali valid
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
    String getFormattedTime();
    String getFormattedDate(char separator = '-');

    // --- Fitur Kontrol & Alarm ---
    /**
     * @brief Cek apakah waktu saat ini cocok dengan waktu alarm (HH:MM:SS).
     * @warning Fungsi ini STATELESS: akan bernilai true berulang kali selama
     * detik yang cocok belum berganti (bisa ratusan kali per detik di dalam loop()).
     * Untuk aksi yang harus terjadi TEPAT SEKALI, gunakan overload di bawah
     * yang menerima parameter `firedFlag`.
     */
    bool isAlarmActive(int hr, int min, int sec = 0);

    /**
     * @brief Versi one-shot dari isAlarmActive(). Aman digunakan untuk aksi
     * yang tidak boleh terulang selama detik yang sama (mis. toggle relay,
     * kirim notifikasi, increment counter).
     *
     * @param firedFlag Variabel bool milik pemanggil (biasanya `static bool`
     * per-alarm) yang dipakai library untuk melacak apakah alarm ini sudah
     * pernah "fire" pada window waktu yang sama. Library akan otomatis
     * me-reset flag ini ketika kondisi waktu sudah tidak cocok lagi, sehingga
     * alarm siap fire lagi di kesempatan berikutnya.
     *
     * Contoh:
     * @code
     * static bool alarmFired = false;
     * if (ntp.isAlarmActive(18, 0, 0, alarmFired)) {
     *   // Hanya dieksekusi SEKALI saat jam 18:00:00 tiba
     * }
     * @endcode
     */
    bool isAlarmActive(int hr, int min, int sec, bool &firedFlag);

    /**
     * @brief Sinkronisasi manual dari variabel eksternal (misal RTC atau Portal)
     */
    void setEpoch(uint32_t manualEpoch);

    void setSyncInterval(uint32_t intervalMs) { _syncInterval = intervalMs; }
};

#endif

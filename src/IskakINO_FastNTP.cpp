#include "IskakINO_FastNTP.h"

/**
 * Konstruktor: Menyiapkan variabel awal.
 * Pointer UDP digunakan agar library fleksibel bisa pakai WiFiUDP atau EthernetUDP.
 */
IskakINO_FastNTP::IskakINO_FastNTP(UDP& udp, const char* server) {
    _udp = &udp;
    _ntpServer = server;
    _syncInterval = 3600000; // Default sinkronisasi ulang setiap 1 jam
    _currentEpoch = 0;
    _lastSyncMs = 0;
    _bootTimestamp = 0;
}

/**
 * begin: Inisialisasi awal.
 * Mengatur offset waktu (misal GMT+7) dan membuka port UDP lokal.
 */
void IskakINO_FastNTP::begin(long gmtOffset, int daylightOffset) {
    _gmtOffsetSec = gmtOffset;
    _daylightOffsetSec = daylightOffset;
    _udp->begin(123); // Membuka port 123 sebagai standar komunikasi NTP
}

/**
 * update: Jantung dari Library (State Machine).
 * Harus dipanggil terus-menerus di loop() tanpa delay.
 */
void IskakINO_FastNTP::update() {
    // 1. CLOCK SIMULATION: Menambah detik secara mandiri tanpa harus tanya server terus.
    if (_currentEpoch > 0) {
        if (millis() - _lastUpdateTick >= 1000) {
            uint32_t diff = (millis() - _lastUpdateTick) / 1000;
            _currentEpoch += diff;
            _lastUpdateTick += diff * 1000; // Menggunakan += untuk akurasi tinggi (mengurangi drift)
        }
    }

    // 2. STATE MACHINE: Mengelola proses request ke server secara asinkron.
    switch (_state) {
        case STATE_IDLE:
            // Cek apakah sudah waktunya sinkronisasi ulang berdasarkan interval
            if (millis() - _lastSyncMs >= _syncInterval || _lastSyncMs == 0) {
                _state = STATE_SEND_REQUEST;
            }
            break;

        case STATE_SEND_REQUEST:
            sendNTPPacket(); // Kirim paket UDP ke server
            _requestMs = millis();
            _state = STATE_AWAIT_RESPONSE;
            break;

        case STATE_AWAIT_RESPONSE:
            // Cek apakah paket balasan sudah datang (Non-blocking parse)
            if (_udp->parsePacket()) {
                _udp->read(_packetBuffer, 48); // Baca 48 byte paket NTP
                
                // Konversi byte 40-43 (Transmit Timestamp) menjadi nilai detik
                uint32_t highWord = word(_packetBuffer[40], _packetBuffer[41]);
                uint32_t lowWord = word(_packetBuffer[42], _packetBuffer[43]);
                uint32_t secsSince1900 = highWord << 16 | lowWord;
                
                // Konversi dari standar NTP (sejak 1900) ke Unix Epoch (sejak 1970)
                const uint32_t seventyYears = 2208988800UL;
                uint32_t epoch = secsSince1900 - seventyYears;
                
                // Update waktu internal dengan offset GMT
                _currentEpoch = epoch + _gmtOffsetSec + _daylightOffsetSec;
                _lastSyncMs = millis();
                _lastUpdateTick = millis();
                
                // Catat waktu "booting" untuk fitur Uptime pada sinkronisasi pertama
                if (_bootTimestamp == 0) _bootTimestamp = _currentEpoch - (millis() / 1000);
                
                _state = STATE_IDLE;
            } 
            // Jika dalam 2 detik tidak ada balasan, anggap RTO (Request Time Out)
            else if (millis() - _requestMs > 2000) { 
                _state = STATE_IDLE;
                // Coba lagi lebih cepat (15 detik) daripada menunggu 1 jam interval normal
                _lastSyncMs = millis() - (_syncInterval - 15000); 
            }
            break;
    }
}

/**
 * sendNTPPacket: Membangun paket data 48 byte sesuai protokol NTP.
 */
void IskakINO_FastNTP::sendNTPPacket() {
    memset(_packetBuffer, 0, 48); // Bersihkan buffer
    _packetBuffer[0] = 0b11100011;   // LI (No Warning), VN (Version 4), Mode (Client)
    _udp->beginPacket(_ntpServer, 123);
    _udp->write(_packetBuffer, 48);
    _udp->endPacket();
}

/**
 * Getters Satuan: Mengambil bagian jam, menit, atau detik saja.
 */
uint32_t IskakINO_FastNTP::getEpoch() { return _currentEpoch; }
int IskakINO_FastNTP::getSeconds()    { return _currentEpoch % 60; }
int IskakINO_FastNTP::getMinutes()    { return (_currentEpoch % 3600) / 60; }
int IskakINO_FastNTP::getHours()      { return (_currentEpoch % 86400L) / 3600; }

/**
 * Logika Kalender: Mengubah Epoch menjadi Tanggal/Bulan/Tahun.
 * Menggunakan gmtime dari <time.h> yang efisien untuk AVR/ESP.
 */
int IskakINO_FastNTP::getDay() {
    tm *ptm = gmtime((const time_t *)&_currentEpoch);
    return ptm->tm_mday;
}

int IskakINO_FastNTP::getMonth() {
    tm *ptm = gmtime((const time_t *)&_currentEpoch);
    return ptm->tm_mon + 1; // tm_mon mulai dari 0 (Januari)
}

int IskakINO_FastNTP::getYear() {
    tm *ptm = gmtime((const time_t *)&_currentEpoch);
    return ptm->tm_year + 1900; // tm_year adalah jumlah tahun sejak 1900
}

/**
 * getDayName: Mengembalikan nama hari dalam Bahasa Indonesia atau Inggris.
 */
String IskakINO_FastNTP::getDayName(NTP_Language lang) {
    // Rumus: Epoch ke hari (0 = Minggu)
    int day = ((_currentEpoch / 86400L) + 4) % 7;
    if (lang == LANG_ID) {
        const char* daysID[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
        return daysID[day];
    }
    const char* daysEN[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return daysEN[day];
}

/**
 * getMonthName: Mengembalikan nama bulan sesuai pilihan bahasa.
 */
String IskakINO_FastNTP::getMonthName(NTP_Language lang) {
    int mon = getMonth() - 1;
    if (lang == LANG_ID) {
        const char* monID[] = {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
        return monID[mon];
    }
    const char* monEN[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    return monEN[mon];
}

/**
 * getFormattedTime: Output string jam standar HH:MM:SS.
 */
String IskakINO_FastNTP::getFormattedTime() {
    char buf[10];
    sprintf(buf, "%02d:%02d:%02d", getHours(), getMinutes(), getSeconds());
    return String(buf);
}

/**
 * getFormattedDate: Output string tanggal DD-MM-YYYY (separator bisa diubah).
 */
String IskakINO_FastNTP::getFormattedDate(char separator) {
    char buf[12];
    sprintf(buf, "%02d%c%02d%c%d", getDay(), separator, getMonth(), separator, getYear());
    return String(buf);
}

/**
 * isAlarmActive: Membandingkan waktu sekarang dengan parameter.
 * Berguna untuk trigger relay/jadwal tertentu.
 */
bool IskakINO_FastNTP::isAlarmActive(int hr, int min, int sec) {
    return (getHours() == hr && getMinutes() == min && getSeconds() == sec);
}

/**
 * isTimeReliable: Mengecek apakah sinkronisasi terakhir masih segar (di bawah x jam).
 * Menghindari penggunaan waktu yang meleset jika internet mati lama.
 */
bool IskakINO_FastNTP::isTimeReliable(uint32_t maxAgeSeconds) {
    return (_currentEpoch > 0 && (millis() - _lastSyncMs < maxAgeSeconds * 1000));
}

/**
 * getUptimeSeconds: Menghitung berapa lama sistem sudah berjalan berdasarkan waktu nyata.
 */
uint32_t IskakINO_FastNTP::getUptimeSeconds() {
    if (_bootTimestamp == 0) return millis() / 1000;
    return _currentEpoch - _bootTimestamp;
}

/**
 * setEpoch: Memasukkan waktu manual (dari RTC eksternal atau portal).
 */
void IskakINO_FastNTP::setEpoch(uint32_t manualEpoch) {
    _currentEpoch = manualEpoch;
    _lastUpdateTick = millis();
}

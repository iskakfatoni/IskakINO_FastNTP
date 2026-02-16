#include "IskakINO_FastNTP.h"

/**
 * Konstruktor: Menyiapkan variabel awal.
 */
IskakINO_FastNTP::IskakINO_FastNTP(UDP& udp, const char* server) {
    _udp = &udp;
    _ntpServer = server;
    _syncInterval = 3600000; // Default 1 jam
    _currentEpoch = 0;
    _lastSyncMs = 0;
    _bootTimestamp = 0;
    _state = STATE_IDLE; // Pastikan state awal didefinisikan
}

/**
 * begin: Inisialisasi awal.
 */
void IskakINO_FastNTP::begin(long gmtOffset, int daylightOffset) {
    _gmtOffsetSec = gmtOffset;
    _daylightOffsetSec = daylightOffset;
    _udp->begin(123); 
}

/**
 * update: Jantung dari Library (State Machine).
 */
void IskakINO_FastNTP::update() {
    // 1. CLOCK SIMULATION
    if (_currentEpoch > 0) {
        if (millis() - _lastUpdateTick >= 1000) {
            uint32_t diff = (millis() - _lastUpdateTick) / 1000;
            _currentEpoch += diff;
            _lastUpdateTick += diff * 1000;
        }
    }

    // 2. STATE MACHINE
    switch (_state) {
        case STATE_IDLE:
            if (millis() - _lastSyncMs >= _syncInterval || _lastSyncMs == 0) {
                _state = STATE_SEND_REQUEST;
            }
            break;

        case STATE_SEND_REQUEST:
            sendNTPPacket();
            _requestMs = millis();
            _state = STATE_AWAIT_RESPONSE;
            break;

        case STATE_AWAIT_RESPONSE:
            if (_udp->parsePacket()) {
                _udp->read(_packetBuffer, 48);
                
                uint32_t highWord = word(_packetBuffer[40], _packetBuffer[41]);
                uint32_t lowWord = word(_packetBuffer[42], _packetBuffer[43]);
                uint32_t secsSince1900 = highWord << 16 | lowWord;
                
                const uint32_t seventyYears = 2208988800UL;
                uint32_t epoch = secsSince1900 - seventyYears;
                
                _currentEpoch = epoch + _gmtOffsetSec + _daylightOffsetSec;
                _lastSyncMs = millis();
                _lastUpdateTick = millis();
                
                if (_bootTimestamp == 0) _bootTimestamp = _currentEpoch - (millis() / 1000);
                
                _state = STATE_IDLE;
            } 
            else if (millis() - _requestMs > 2000) { 
                _state = STATE_IDLE;
                _lastSyncMs = millis() - (_syncInterval - 15000); 
            }
            break;
    }
}

void IskakINO_FastNTP::sendNTPPacket() {
    memset(_packetBuffer, 0, 48);
    _packetBuffer[0] = 0b11100011;   
    _udp->beginPacket(_ntpServer, 123);
    _udp->write(_packetBuffer, 48);
    _udp->endPacket();
}

// --- FUNGSI YANG DIPERBAIKI/DITAMBAHKAN UNTUK EXAMPLE 09 ---

/**
 * getMillisSinceLastSync: Menghitung selisih waktu sejak sinkronisasi sukses terakhir.
 */
uint32_t IskakINO_FastNTP::getMillisSinceLastSync() {
    if (_lastSyncMs == 0) return 0;
    return (uint32_t)(millis() - _lastSyncMs);
}

// --- Getters ---
uint32_t IskakINO_FastNTP::getEpoch() { return _currentEpoch; }
int IskakINO_FastNTP::getSeconds()    { return _currentEpoch % 60; }
int IskakINO_FastNTP::getMinutes()    { return (_currentEpoch % 3600) / 60; }
int IskakINO_FastNTP::getHours()      { return (_currentEpoch % 86400L) / 3600; }

int IskakINO_FastNTP::getDay() {
    tm *ptm = gmtime((const time_t *)&_currentEpoch);
    return ptm->tm_mday;
}

int IskakINO_FastNTP::getMonth() {
    tm *ptm = gmtime((const time_t *)&_currentEpoch);
    return ptm->tm_mon + 1;
}

int IskakINO_FastNTP::getYear() {
    tm *ptm = gmtime((const time_t *)&_currentEpoch);
    return ptm->tm_year + 1900;
}

String IskakINO_FastNTP::getDayName(NTP_Language lang) {
    int day = ((_currentEpoch / 86400L) + 4) % 7;
    if (lang == LANG_ID) {
        const char* daysID[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
        return daysID[day];
    }
    const char* daysEN[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return daysEN[day];
}

String IskakINO_FastNTP::getMonthName(NTP_Language lang) {
    int mon = getMonth() - 1;
    if (lang == LANG_ID) {
        const char* monID[] = {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
        return monID[mon];
    }
    const char* monEN[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    return monEN[mon];
}

String IskakINO_FastNTP::getFormattedTime() {
    char buf[10];
    sprintf(buf, "%02d:%02d:%02d", getHours(), getMinutes(), getSeconds());
    return String(buf);
}

String IskakINO_FastNTP::getFormattedDate(char separator) {
    char buf[12];
    sprintf(buf, "%02d%c%02d%c%d", getDay(), separator, getMonth(), separator, getYear());
    return String(buf);
}

bool IskakINO_FastNTP::isAlarmActive(int hr, int min, int sec) {
    return (getHours() == hr && getMinutes() == min && getSeconds() == sec);
}

bool IskakINO_FastNTP::isTimeReliable(uint32_t maxAgeSeconds) {
    return (_currentEpoch > 0 && (millis() - _lastSyncMs < maxAgeSeconds * 1000));
}

uint32_t IskakINO_FastNTP::getUptimeSeconds() {
    if (_bootTimestamp == 0) return millis() / 1000;
    return _currentEpoch - _bootTimestamp;
}

void IskakINO_FastNTP::setEpoch(uint32_t manualEpoch) {
    _currentEpoch = manualEpoch;
    _lastUpdateTick = millis();
    _lastSyncMs = millis(); // Anggap ini sebagai titik sinkronisasi manual
}

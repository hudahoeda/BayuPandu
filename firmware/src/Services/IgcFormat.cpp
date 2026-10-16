#include "IgcFormat.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace igc {

namespace {

// Degrees and thousandths of a minute, rounded, carrying 60.000' into a degree.
void splitAngle(double value, unsigned& degrees, unsigned& milliMinutes) {
    double a = std::fabs(value);
    degrees = static_cast<unsigned>(a);
    milliMinutes = static_cast<unsigned>(std::lround((a - degrees) * 60000.0));
    if (milliMinutes >= 60000) {
        degrees++;
        milliMinutes -= 60000;
    }
}

// Five characters: 00123, or -0012 below sea level.
void formatAltitude(char* out, int metres) {
    if (metres < -9999) metres = -9999;
    if (metres > 99999) metres = 99999;
    if (metres < 0) {
        std::snprintf(out, 6, "-%04d", -metres);
    } else {
        std::snprintf(out, 6, "%05d", metres);
    }
}

size_t finish(int n, size_t size) {
    return (n < 0 || static_cast<size_t>(n) >= size) ? 0 : static_cast<size_t>(n);
}

bool reserved(char c) {
    // Characters the IGC format keeps for itself, plus anything not printable.
    return c < 0x20 || c > 0x7E || std::strchr("$*!\\^~", c) != nullptr;
}

void copyValue(char* dst, size_t size, const char* begin, const char* end) {
    while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) begin++;
    while (end > begin && std::isspace(static_cast<unsigned char>(end[-1]))) end--;
    size_t n = 0;
    for (const char* p = begin; p < end && n + 1 < size; p++) {
        if (!reserved(*p)) dst[n++] = *p;
    }
    dst[n] = '\0';
}

bool keyIs(const char* begin, const char* end, const char* key) {
    while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) begin++;
    while (end > begin && std::isspace(static_cast<unsigned char>(end[-1]))) end--;
    size_t len = std::strlen(key);
    if (static_cast<size_t>(end - begin) != len) return false;
    for (size_t i = 0; i < len; i++) {
        if (std::toupper(static_cast<unsigned char>(begin[i])) != key[i]) return false;
    }
    return true;
}

}  // namespace

size_t formatBRecord(char* out, size_t size, const Fix& fix) {
    unsigned latDeg, latMin, lonDeg, lonMin;
    splitAngle(fix.latitude, latDeg, latMin);
    splitAngle(fix.longitude, lonDeg, lonMin);
    uint32_t s = (fix.utcMs / 1000) % 86400;
    char pressure[6], gnss[6];
    formatAltitude(pressure, fix.pressureAltitude);
    formatAltitude(gnss, fix.gnssAltitude);
    int n = std::snprintf(out, size, "B%02u%02u%02u%02u%05u%c%03u%05u%c%c%s%s\r\n",
                          static_cast<unsigned>(s / 3600), static_cast<unsigned>(s / 60 % 60),
                          static_cast<unsigned>(s % 60), latDeg, latMin, fix.latitude < 0 ? 'S' : 'N',
                          lonDeg, lonMin, fix.longitude < 0 ? 'W' : 'E', fix.valid3d ? 'A' : 'V',
                          pressure, gnss);
    return finish(n, size);
}

size_t formatHeader(char* out, size_t size, const Header& h) {
    static const PilotInfo kNobody;
    const PilotInfo& p = h.pilot ? *h.pilot : kNobody;
    int n = std::snprintf(out, size,
                          "A%s%s BayuPandu\r\n"
                          "HFDTEDATE:%02u%02u%02u,%02d\r\n"
                          "HFPLTPILOTINCHARGE:%s\r\n"
                          "HFCM2CREW2:NIL\r\n"
                          "HFGTYGLIDERTYPE:%s\r\n"
                          "HFGIDGLIDERID:%s\r\n"
                          "HFDTMGPSDATUM:WGS84\r\n"
                          "HFRFWFIRMWAREVERSION:%s\r\n"
                          "HFRHWHARDWAREVERSION:%s\r\n"
                          "HFFTYFRTYPE:BayuPandu,BayuPandu\r\n"
                          "HFGPSRECEIVER:u-blox,NEO-6M,50,50000\r\n"
                          "HFPRSPRESSALTSENSOR:MEAS,MS5611,25000\r\n"
                          "HFALGALTGPS:GEO\r\n"
                          "HFALPALTPRESSURE:ISA\r\n",
                          kManufacturer, h.serial, h.date.day, h.date.month, h.date.year % 100,
                          h.flightOfDay, p.pilot, p.gliderType, p.gliderId, h.firmware, h.hardware);
    return finish(n, size);
}

size_t formatFileName(char* out, size_t size, const char* dir, const Date& date, const char* serial,
                      int flightOfDay) {
    int n = std::snprintf(out, size, "%s/%04u-%02u-%02u-%s-%s-%02d.igc", dir, date.year, date.month,
                          date.day, kManufacturer, serial, flightOfDay);
    return finish(n, size);
}

void parsePilotFile(const char* text, PilotInfo& info) {
    if (!text) return;
    const char* line = text;
    while (*line) {
        const char* end = line;
        while (*end && *end != '\n') end++;
        const char* stop = end;
        for (const char* c = line; c < end; c++) {
            if (*c == '#') {
                stop = c;
                break;
            }
        }
        const char* eq = line;
        while (eq < stop && *eq != '=') eq++;
        if (eq < stop) {
            if (keyIs(line, eq, "PILOT")) copyValue(info.pilot, sizeof(info.pilot), eq + 1, stop);
            else if (keyIs(line, eq, "GLIDER")) copyValue(info.gliderType, sizeof(info.gliderType), eq + 1, stop);
            else if (keyIs(line, eq, "GLIDER_ID")) copyValue(info.gliderId, sizeof(info.gliderId), eq + 1, stop);
        }
        line = *end ? end + 1 : end;
    }
}

}  // namespace igc

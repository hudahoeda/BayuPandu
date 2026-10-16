#pragma once

#include <cstddef>
#include <cstdint>

// Text of an IGC flight log (FAI IGC "Technical Specification for IGC-approved
// GNSS flight recorders", records A, H and B), kept free of I/O so it can be
// tested on the host. Lines end in CR LF, as the specification requires.
namespace igc {

// Manufacturer code. A leading X marks a recorder that is not IGC-approved.
constexpr const char* kManufacturer = "XBP";

struct Fix {
    uint32_t utcMs = 0;      // ms since midnight UTC
    double latitude = 0.0;   // degrees, north positive
    double longitude = 0.0;  // degrees, east positive
    bool valid3d = false;    // 'A' when the GNSS has a 3D fix, else 'V'
    int pressureAltitude = 0;  // m, ISA (1013.25 hPa)
    int gnssAltitude = 0;      // m
};

struct Date {
    uint16_t year = 0;  // 0 when unknown
    uint8_t month = 0;
    uint8_t day = 0;
};

// Pilot and glider, read from pilot.txt on the card.
struct PilotInfo {
    char pilot[41] = "";
    char gliderType[41] = "";
    char gliderId[41] = "";
};

struct Header {
    const char* serial = "000";   // 3 characters, unique per device
    Date date;
    int flightOfDay = 1;
    const PilotInfo* pilot = nullptr;
    const char* firmware = "";
    const char* hardware = "";
};

// Each returns the length written (excluding the terminator), or 0 when `size`
// is too small.
size_t formatBRecord(char* out, size_t size, const Fix& fix);
size_t formatHeader(char* out, size_t size, const Header& header);
// "<dir>/2026-10-16-XBP-SIM-01.igc", the IGC long file name.
size_t formatFileName(char* out, size_t size, const char* dir, const Date& date, const char* serial,
                      int flightOfDay);

// Reads PILOT=, GLIDER= and GLIDER_ID= lines (any case, # starts a comment).
// Values are trimmed, cut to 40 characters, and stripped of characters the IGC
// format reserves. Unknown keys are ignored.
void parsePilotFile(const char* text, PilotInfo& info);

}  // namespace igc

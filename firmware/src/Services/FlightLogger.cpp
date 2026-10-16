#include "FlightLogger.h"
#include "version.h"
#include <cmath>
#include <cstring>
#include <string>

// The ESP32 toolchain builds C++11, where these need a definition of their own.
constexpr const char* FlightLogger::kDir;
constexpr const char* FlightLogger::kPilotFile;

FlightLogger::FlightLogger(IStorage& storage) : storage(storage) {}

void FlightLogger::setDevice(const char* id, const char* hw) {
    std::strncpy(serial, id, 3);
    serial[3] = '\0';
    hardware = hw;
}

bool FlightLogger::initialize() {
    wasStarted = true;
    cardOk = storage.isHealthy() && storage.makeDir(kDir);
    pilot = igc::PilotInfo();
    if (cardOk && storage.fileExists(kPilotFile)) {
        char text[512] = "";
        if (storage.readFile(kPilotFile, text, sizeof(text))) igc::parsePilotFile(text, pilot);
    }
    return cardOk;
}

void FlightLogger::update(const FlightData& data, uint32_t nowMs) {
    if (!wasStarted) return;
    const GPSData& gps = data.gpsData;
    // One record per GPS second, stamped with that second. If the receiver stops
    // sending time, carry on from the clock so the log has no hole.
    uint32_t utcMs;
    if (gps.timestamp != 0 && (!sampled || gps.timestamp / 1000 != lastFix.utcMs / 1000) &&
        nowMs - lastSampleMs >= 500) {
        utcMs = gps.timestamp / 1000 * 1000;
    } else if (sampled && nowMs - lastSampleMs >= 1500) {
        utcMs = lastFix.utcMs + 1000;
    } else {
        return;
    }
    lastSampleMs = nowMs;
    sampled = true;

    igc::Fix fix;
    fix.utcMs = utcMs;
    fix.valid3d = gps.hasValidFix;
    fix.latitude = gps.hasValidFix ? gps.latitude : lastFix.latitude;
    fix.longitude = gps.hasValidFix ? gps.longitude : lastFix.longitude;
    fix.gnssAltitude = gps.hasValidFix ? static_cast<int>(std::lround(gps.altitude)) : 0;
    fix.pressureAltitude = data.isValid ? static_cast<int>(std::lround(data.altitude)) : 0;
    lastFix = fix;
    if (gps.hasValidFix) {
        haveFix = true;
        if (gps.year != 0) {
            date.year = gps.year;
            date.month = gps.month;
            date.day = gps.day;
        }
    }

    if (!recording) {
        if (!gps.hasValidFix) {
            fastSeconds = 0;
            return;
        }
        preTakeoff[preHead] = fix;
        preHead = (preHead + 1) % kPreTakeoffS;
        if (preCount < kPreTakeoffS) preCount++;
        fastSeconds = gps.speed > kTakeoffSpeed ? fastSeconds + 1 : 0;
        if (fastSeconds >= kTakeoffHoldS) {
            recordStartMs = nowMs;
            openFile();
        }
        return;
    }

    if (!haveFix) return;  // nothing to place a record at yet
    char line[48];
    if (igc::formatBRecord(line, sizeof(line), fix)) append(line);

    if (gps.hasValidFix) {
        bool still = gps.speed < kLandedSpeed && std::fabs(data.verticalSpeed) < kLandedClimb;
        stillSeconds = still ? stillSeconds + 1 : 0;
        if (stillSeconds >= kLandingS) endFlight();
    }
}

void FlightLogger::openFile() {
    recording = true;
    stillSeconds = 0;
    writeOk = cardOk;
    if (!cardOk) return;

    int flight = 1;
    for (; flight <= 99; flight++) {
        igc::formatFileName(fileName, sizeof(fileName), kDir, date, serial, flight);
        if (!storage.fileExists(fileName)) break;
    }
    if (flight > 99) {
        writeOk = false;
        return;
    }

    char header[768];
    igc::Header h;
    h.serial = serial;
    h.date = date;
    h.flightOfDay = flight;
    h.pilot = &pilot;
    h.firmware = BAYUPANDU_VERSION;
    h.hardware = hardware;
    if (!igc::formatHeader(header, sizeof(header), h) || !storage.writeFile(fileName, header)) {
        writeOk = false;
        return;
    }

    // The kept fixes, oldest first, in one write; the newest is the takeoff fix.
    std::string kept;
    kept.reserve(preCount * 37);
    char line[48];
    for (int i = 0; i < preCount; i++) {
        const igc::Fix& f = preTakeoff[(preHead - preCount + i + kPreTakeoffS) % kPreTakeoffS];
        if (igc::formatBRecord(line, sizeof(line), f)) kept += line;
    }
    append(kept.c_str());
    preCount = 0;
}

bool FlightLogger::append(const char* text) {
    if (!cardOk) return false;
    writeOk = storage.appendFile(fileName, text);
    return writeOk;
}

void FlightLogger::endFlight() {
    recording = false;
    fastSeconds = 0;
    stillSeconds = 0;
    preCount = 0;
    fileName[0] = '\0';
}

FlightLogger::Status FlightLogger::getStatus() const {
    if (!cardOk) return Status::NoCard;
    if (recording) return writeOk ? Status::Recording : Status::WriteFailed;
    return Status::Ready;
}

uint32_t FlightLogger::getRecordingSeconds(uint32_t nowMs) const {
    return recording ? (nowMs - recordStartMs) / 1000 : 0;
}

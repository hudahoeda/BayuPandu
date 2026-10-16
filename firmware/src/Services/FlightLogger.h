#pragma once

#include "HAL/IStorage.h"
#include "Data/Types.h"
#include "Services/IgcFormat.h"
#include <cstdint>

// Writes each flight to the card as an IGC file, by itself:
//  - While on the ground it keeps the last kPreTakeoffS seconds of fixes.
//  - Takeoff is kTakeoffHoldS seconds in a row above kTakeoffSpeed ground speed;
//    the file then opens with the kept fixes, so the launch run is in it.
//  - From then on one B-record a second goes straight to the card, so a flat
//    battery loses at most the last second.
//  - Landing is kLandingS seconds in a row slow and level; the file is closed.
// Pilot and glider for the header come from /pilot.txt on the card.
class FlightLogger {
public:
    enum class Status { NoCard, Ready, Recording, WriteFailed };

    static constexpr const char* kDir = "/flights";
    static constexpr const char* kPilotFile = "/pilot.txt";
    static constexpr int kPreTakeoffS = 60;
    static constexpr int kTakeoffHoldS = 5;
    static constexpr float kTakeoffSpeed = 4.0f;   // m/s ground speed
    static constexpr int kLandingS = 60;
    static constexpr float kLandedSpeed = 1.5f;    // m/s ground speed
    static constexpr float kLandedClimb = 0.5f;    // m/s either way

    explicit FlightLogger(IStorage& storage);

    // A 3-character id unique to this device, and what it runs on, for the header.
    void setDevice(const char* serial, const char* hardware);
    // Checks the card, makes the flights folder and reads pilot.txt.
    bool initialize();
    // Call every loop; samples once a second.
    void update(const FlightData& data, uint32_t nowMs);
    // Closes the current flight now, e.g. when a replay starts over.
    void endFlight();

    Status getStatus() const;
    // Seconds since the file was opened, while recording.
    uint32_t getRecordingSeconds(uint32_t nowMs) const;
    // The open file's path, or "" when not recording.
    const char* getFileName() const { return recording ? fileName : ""; }
    const igc::PilotInfo& getPilot() const { return pilot; }

private:
    void openFile();
    bool append(const char* text);

    IStorage& storage;
    bool cardOk = false;
    bool writeOk = true;
    bool recording = false;
    bool wasStarted = false;  // initialize() has run
    uint32_t lastSampleMs = 0;
    bool sampled = false;
    uint32_t recordStartMs = 0;
    int fastSeconds = 0;
    int stillSeconds = 0;

    igc::Fix preTakeoff[kPreTakeoffS];
    int preHead = 0;   // next slot to write
    int preCount = 0;
    igc::Fix lastFix;  // position repeated in 'V' records after the fix is lost
    bool haveFix = false;
    igc::Date date;

    igc::PilotInfo pilot;
    char serial[4] = "000";
    const char* hardware = "";
    char fileName[48] = "";
};

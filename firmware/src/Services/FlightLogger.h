#pragma once

#include "HAL/IStorage.h"
#include "Data/Types.h"

// FlightLogger handles IGC logging, automatic detection, and file rotation
class FlightLogger {
public:
    FlightLogger(IStorage& storage);
    // Call once at startup
    bool initialize();
    // Call in loop to check flight status and append data
    void update(const FlightData& data, FlightState state);
    // Manually start or stop logging
    void startLogging();
    void stopLogging();
private:
    IStorage& storage;
    bool loggingActive;
    uint32_t lastLogTime;
    uint16_t logInterval;
    const char* currentFilename;
    // Internal methods
    void openLogFile();
    void closeLogFile();
    void writeIGCHeader();
    void writeIGCRecord(const FlightData& data);
    void rotateFilesIfNeeded();
    bool detectFlightStart(FlightState state);
    bool detectFlightEnd(FlightState state);
};

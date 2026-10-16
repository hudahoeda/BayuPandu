#pragma once

#include "Data/Types.h"
#include <vector>
#include <string>

// Structure to hold a single IGC record (B-record equivalent)
struct IGCRecord {
    uint32_t timestamp; // Time in milliseconds since start of day (00:00:00)
    double latitude;    // degrees
    double longitude;   // degrees
    float gpsAltitude;  // meters GPS altitude
    float baroAltitude; // meters barometric altitude
    float speed;        // m/s (GPS ground speed), derived from consecutive fixes
    float heading;      // degrees (GPS track angle), derived from consecutive fixes
    float climbRate;    // m/s, baro altitude change smoothed over a few records
    float smoothAltitude; // m, baro altitude averaged over the same window
    uint8_t numSatellites;
    float hdop;         // horizontal dilution of precision
};

class IGCParser {
public:
    IGCParser();
    
    // Loads and parses an IGC file from the given path.
    bool loadFromFile(const std::string& filePath);

    // Returns the number of records loaded.
    size_t getRecordCount() const;

    // Returns a specific record by index.
    // Returns an empty record if index is out of bounds.
    IGCRecord getRecord(size_t index) const;

    // Returns all records.
    const std::vector<IGCRecord>& getAllRecords() const;

    // Checks if the parser has loaded any valid data.
    bool hasData() const;

private:
    std::vector<IGCRecord> records;

    // Helper to parse a single B-record line
    bool parseBRecord(const std::string& line, IGCRecord& record);
    // Fills speed, heading and climb rate, which B-records don't carry.
    void deriveMotion();
    double parseLatLon(const std::string& latLonStr);
    uint32_t parseTime(const std::string& timeStr);
};

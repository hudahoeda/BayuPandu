#include "IGCParser.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm> // For std::remove

IGCParser::IGCParser() : records() {}

bool IGCParser::loadFromFile(const std::string& filePath) {
    records.clear(); // Clear any previous data
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // Handle error: file not found or cannot be opened
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.length() > 0 && line[0] == 'B') { // B-record for flight data
            IGCRecord record;
            if (parseBRecord(line, record)) {
                records.push_back(record);
            }
        }
    }
    file.close();
    return !records.empty();
}

size_t IGCParser::getRecordCount() const {
    return records.size();
}

IGCRecord IGCParser::getRecord(size_t index) const {
    if (index < records.size()) {
        return records[index];
    }
    return {}; // Return empty/default record if out of bounds
}

const std::vector<IGCRecord>& IGCParser::getAllRecords() const {
    return records;
}

bool IGCParser::hasData() const {
    return !records.empty();
}

bool IGCParser::parseBRecord(const std::string& line, IGCRecord& record) {
    if (line.length() < 35) { // Minimum length for essential B-record data
        return false;
    }

    try {
        // Time HHMMSS
        record.timestamp = parseTime(line.substr(1, 6));

        // Latitude DDMMmmmN/S
        record.latitude = parseLatLon(line.substr(7, 8));

        // Longitude DDDMMmmmE/W
        record.longitude = parseLatLon(line.substr(15, 9));

        // Fix validity 'A' or 'V' - not directly used in IGCRecord but good to know
        char fixValidity = line[24]; 
        // For simplicity, we assume 'A' fix for simulation purposes if present.
        // IGCRecord does not have a 'hasValidFix' field directly, but FlightData will use it.

        // Pressure Altitude EEEEE
        record.baroAltitude = std::stof(line.substr(25, 5));

        // GPS Altitude nnnnn
        record.gpsAltitude = std::stof(line.substr(30, 5));

        // Optional fields (speed, heading, satellites, HDOP) - these might not always be present or standardized
        // IGC spec is complex; these are often in 'K' records or manufacturer extensions.
        // For basic simulation, we might need to derive speed/heading from successive lat/lon or add more robust parsing.
        // For now, setting to defaults or simple parsing if format is consistent.
        
        // Example: If speed and heading are present in a known format (e.g., after 35 chars)
        // This part needs to be adapted based on actual IGC file examples if more detail is needed.
        // For now, let's assume speed and heading are not directly in the B-record we're parsing.
        record.speed = 0.0f; // To be derived or set from more complete IGC parsing
        record.heading = 0.0f;
        record.numSatellites = 0;
        record.hdop = 99.9f;

        return true;
    } catch (const std::exception& e) {
        // Parsing error
        return false;
    }
}

double IGCParser::parseLatLon(const std::string& latLonStr) {
    // Format: DDMMmmmN/S or DDDMMmmmE/W
    // Example: 3357123N -> 33 degrees, 57.123 minutes North
    // Example: 00730456W -> 7 degrees, 30.456 minutes West
    
    std::string s = latLonStr;
    char hemisphere = s.back();
    s.pop_back(); // Remove N/S/E/W
    
    int degrees;
    double minutes;

    if (hemisphere == 'N' || hemisphere == 'S') { // Latitude
        degrees = std::stoi(s.substr(0, 2));
        minutes = std::stod(s.substr(2, 5)) / 1000.0;
    } else { // Longitude
        degrees = std::stoi(s.substr(0, 3));
        minutes = std::stod(s.substr(3, 5)) / 1000.0;
    }

    double decimalDegrees = degrees + (minutes / 60.0);

    if (hemisphere == 'S' || hemisphere == 'W') {
        decimalDegrees *= -1.0;
    }
    return decimalDegrees;
}

uint32_t IGCParser::parseTime(const std::string& timeStr) {
    // Format: HHMMSS
    // Returns time in milliseconds from start of day
    int hours = std::stoi(timeStr.substr(0, 2));
    int minutes = std::stoi(timeStr.substr(2, 2));
    int seconds = std::stoi(timeStr.substr(4, 2));
    return (hours * 3600 + minutes * 60 + seconds) * 1000;
}

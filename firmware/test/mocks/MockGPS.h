#ifndef MOCK_GPS_H
#define MOCK_GPS_H

#include "HAL/IGPS.h"

class MockGPS : public IGPS {
public:
    // --- Test Control Methods ---
    void setNextPosition(const GPSData& data) { data_ = data; }
    void setFixStatus(bool hasFix) { data_.hasValidFix = hasFix; }

    // --- IGPS Implementation ---
    bool initialize() override {
        initialize_called_ = true;
        return true;
    }

    bool update() override {
        update_call_count_++;
        // In a real scenario, this would parse data. Here, we just increment a counter.
        return true;
    }

    bool hasValidFix() override {
        return data_.hasValidFix;
    }

    GPSData getCurrentPosition() override {
        return data_;
    }

    uint8_t getSatelliteCount() override {
        return data_.satellites;
    }

    float getHDOP() override {
        return data_.hdop;
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    int getUpdateCallCount() const { return update_call_count_; }
    void reset() {
        data_ = GPSData{};
        initialize_called_ = false;
        update_call_count_ = 0;
    }

private:
    GPSData data_{};
    bool initialize_called_ = false;
    int update_call_count_ = 0;
};

#endif // MOCK_GPS_H

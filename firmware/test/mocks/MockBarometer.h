#ifndef MOCK_BAROMETER_H
#define MOCK_BAROMETER_H

#include "HAL/IBarometer.h"
#include <cmath>

class MockBarometer : public IBarometer {
public:
    // --- Test Control Methods ---
    void setNextPressure(float pressure) { pressure_ = pressure; }
    void setNextTemperature(float temp) { temperature_ = temp; }
    void setHealthStatus(bool isHealthy) { is_healthy_ = isHealthy; }

    // --- IBarometer Implementation ---
    bool initialize() override {
        initialize_called_ = true;
        return true;
    }

    bool readPressure(float& pressure) override {
        readPressure_call_count_++;
        pressure = pressure_;
        return is_healthy_;
    }

    bool readTemperature(float& temperature) override {
        readTemperature_call_count_++;
        temperature = temperature_;
        return is_healthy_;
    }

    float calculateAltitude(float pressure, float seaLevelPressure = 1013.25f) override {
        // Standard formula implementation from https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
        return ((pow((seaLevelPressure / pressure), 1/5.257) - 1.0) * (temperature_ + 273.15)) / 0.0065;
    }

    bool isHealthy() override {
        return is_healthy_;
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    int getReadPressureCallCount() const { return readPressure_call_count_; }
    int getReadTemperatureCallCount() const { return readTemperature_call_count_; }
    void reset() {
        pressure_ = 1013.25f;
        temperature_ = 25.0f;
        is_healthy_ = true;
        initialize_called_ = false;
        readPressure_call_count_ = 0;
        readTemperature_call_count_ = 0;
    }


private:
    float pressure_ = 1013.25f;
    float temperature_ = 25.0f;
    bool is_healthy_ = true;

    bool initialize_called_ = false;
    int readPressure_call_count_ = 0;
    int readTemperature_call_count_ = 0;
};

#endif // MOCK_BAROMETER_H

#ifndef MOCK_POWER_H
#define MOCK_POWER_H

#include "HAL/IPower.h"

class MockPower : public IPower {
public:
    // --- Test Control Methods ---
    void setVoltage(float voltage) { voltage_ = voltage; }
    void setChargingStatus(bool isCharging) { is_charging_ = isCharging; }

    // --- IPower Implementation ---
    bool initialize() override {
        initialize_called_ = true;
        return true;
    }

    float getBatteryVoltage() override {
        getBatteryVoltage_call_count_++;
        return voltage_;
    }

    uint8_t getBatteryPercentage() override {
        getBatteryPercentage_call_count_++;
        // Simple linear mapping for testing purposes
        if (voltage_ >= 4.2f) return 100;
        if (voltage_ <= 3.4f) return 0;
        return static_cast<uint8_t>((voltage_ - 3.4f) / (4.2f - 3.4f) * 100.0f);
    }

    bool isCharging() override {
        isCharging_call_count_++;
        return is_charging_;
    }

    void enterLowPowerMode() override {
        enterLowPowerMode_call_count_++;
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    int getBatteryVoltageCallCount() const { return getBatteryVoltage_call_count_; }
    int getBatteryPercentageCallCount() const { return getBatteryPercentage_call_count_; }
    int getIsChargingCallCount() const { return isCharging_call_count_; }
    int getEnterLowPowerModeCallCount() const { return enterLowPowerMode_call_count_; }
    void reset() {
        voltage_ = 4.2f;
        is_charging_ = false;
        initialize_called_ = false;
        getBatteryVoltage_call_count_ = 0;
        getBatteryPercentage_call_count_ = 0;
        isCharging_call_count_ = 0;
        enterLowPowerMode_call_count_ = 0;
    }

private:
    float voltage_ = 4.2f;
    bool is_charging_ = false;
    bool initialize_called_ = false;
    int getBatteryVoltage_call_count_ = 0;
    int getBatteryPercentage_call_count_ = 0;
    int isCharging_call_count_ = 0;
    int enterLowPowerMode_call_count_ = 0;
};

#endif // MOCK_POWER_H

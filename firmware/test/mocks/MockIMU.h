#ifndef MOCK_IMU_H
#define MOCK_IMU_H

#include "HAL/IIMU.h"

class MockIMU : public IIMU {
public:
    // --- Test Control Methods ---
    void setNextAcceleration(const Vector3& accel) { attitude_.acceleration = accel; }
    void setNextGyroscope(const Vector3& gyro) { attitude_.angularVelocity = gyro; }
    void setNextAttitude(const AttitudeData& attitude) { attitude_ = attitude; }
    void setCalibrationStatus(bool isCalibrated) { attitude_.isCalibrated = isCalibrated; }

    // --- IIMU Implementation ---
    bool initialize() override {
        initialize_called_ = true;
        return true;
    }

    bool calibrate() override {
        calibrate_called_ = true;
        attitude_.isCalibrated = true; // Assume calibration is successful
        return true;
    }

    bool readAcceleration(Vector3& accel) override {
        readAcceleration_call_count_++;
        accel = attitude_.acceleration;
        return true;
    }

    bool readGyroscope(Vector3& gyro) override {
        readGyroscope_call_count_++;
        gyro = attitude_.angularVelocity;
        return true;
    }

    bool readMagnetometer(Vector3& mag) override {
        readMagnetometer_call_count_++;
        // Return a dummy value for now
        mag.x = 1.0f;
        mag.y = 0.0f;
        mag.z = 0.0f;
        return true;
    }

    AttitudeData getAttitude() override {
        return attitude_;
    }

    bool isCalibrated() override {
        return attitude_.isCalibrated;
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    bool wasCalibrateCalled() const { return calibrate_called_; }
    int getReadAccelerationCallCount() const { return readAcceleration_call_count_; }
    int getReadGyroscopeCallCount() const { return readGyroscope_call_count_; }
    int getReadMagnetometerCallCount() const { return readMagnetometer_call_count_; }
    void reset() {
        attitude_ = AttitudeData{};
        initialize_called_ = false;
        calibrate_called_ = false;
        readAcceleration_call_count_ = 0;
        readGyroscope_call_count_ = 0;
        readMagnetometer_call_count_ = 0;
    }

private:
    AttitudeData attitude_{};
    bool initialize_called_ = false;
    bool calibrate_called_ = false;
    int readAcceleration_call_count_ = 0;
    int readGyroscope_call_count_ = 0;
    int readMagnetometer_call_count_ = 0;
};

#endif // MOCK_IMU_H

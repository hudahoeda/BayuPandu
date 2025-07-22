#pragma once

#include "IIMU.h"

class IMUImpl : public IIMU {
public:
    bool initialize() override { return true; }
    bool calibrate() override { return true; }
    bool readAcceleration(Vector3& accel) override { accel = Vector3{}; return true; }
    bool readGyroscope(Vector3& gyro) override { gyro = Vector3{}; return true; }
    bool readMagnetometer(Vector3& mag) override { mag = Vector3{}; return true; }
    AttitudeData getAttitude() override { return AttitudeData{}; }
    bool isCalibrated() override { return true; }
};

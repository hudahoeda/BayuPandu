#ifndef IIMU_H
#define IIMU_H

#include "Data/Types.h"

class IIMU {
public:
    virtual ~IIMU() = default;

    virtual bool initialize() = 0;
    virtual bool calibrate() = 0;
    virtual bool readAcceleration(Vector3& accel) = 0;
    virtual bool readGyroscope(Vector3& gyro) = 0;
    virtual bool readMagnetometer(Vector3& mag) = 0;
    virtual AttitudeData getAttitude() = 0;
    virtual bool isCalibrated() = 0;
};

#endif // IIMU_H

#pragma once

#include "HAL/IIMU.h"
#include "Data/Types.h"

class IMUService
{
public:
    IMUService(IIMU& imu);

    void update();

    AttitudeData getAttitudeData() const;

private:
    IIMU& imu;
    AttitudeData attitudeData;
};

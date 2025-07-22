#include "IMUService.h"

IMUService::IMUService(IIMU& imu)
    : imu(imu)
{
}

void IMUService::update()
{
    // In a real implementation, we would read raw sensor data
    // and perform sensor fusion here.
    // For now, we just get the attitude from the mock.
    attitudeData = imu.getAttitude();
}

AttitudeData IMUService::getAttitudeData() const
{
    return attitudeData;
}

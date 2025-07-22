#pragma once

#include "HAL/IBarometer.h"
#include "HAL/IAudio.h"
#include "HAL/IArduino.h"

class VariometerService
{
public:
    VariometerService(IBarometer& barometer, IAudio& audio, IArduino& arduino);

    void update();

    float getVerticalSpeed() const;

private:
    IBarometer& barometer;
    IAudio& audio;
    IArduino& arduino;

    float verticalSpeed;

    // Kalman filter state
    float q; // process noise covariance
    float r; // measurement noise covariance
    float p; // estimation error covariance
    float k; // kalman gain
    float x; // value
};

#include "VariometerService.h"

VariometerService::VariometerService(IBarometer& barometer, IAudio& audio, IArduino& arduino)
    : barometer(barometer),
      audio(audio),
      arduino(arduino),
      verticalSpeed(0.0f),
      q(0.00001f), // process noise covariance
      r(0.01f),   // measurement noise covariance
      p(1.0f),    // estimation error covariance
      k(0.0f),    // kalman gain
      x(0.0f)     // value
{
    // Initialize altitude value
    float pressure;
    if (barometer.readPressure(pressure))
    {
        x = barometer.calculateAltitude(pressure);
    }
}

void VariometerService::update()
{
    static unsigned long lastTime = 0;
    unsigned long currentTime = arduino.millis();
    float dt = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    if (dt > 0)
    {
        float pressure;
        float altitude = x; // Default to previous altitude
        if (barometer.readPressure(pressure))
        {
            altitude = barometer.calculateAltitude(pressure);
        }

        // Kalman filter prediction
        p = p + q;

        // Kalman filter update
        k = p / (p + r);
        x = x + k * (altitude - x);
        p = (1 - k) * p;

        verticalSpeed = (x - verticalSpeed) / dt;

        // Simple audio logic
        if (verticalSpeed > 0.5)
        {
            audio.tone(1000 + verticalSpeed * 100, 100);
        }
        else if (verticalSpeed < -0.5)
        {
            audio.tone(500 + verticalSpeed * 50, 100);
        }
        else
        {
            audio.noTone();
        }
    }
}

float VariometerService::getVerticalSpeed() const
{
    return verticalSpeed;
}

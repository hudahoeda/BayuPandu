#ifndef IBADOMETER_H
#define IBADOMETER_H

class IBarometer {
public:
    virtual ~IBarometer() = default;

    virtual bool initialize() = 0;
    virtual bool readPressure(float& pressure) = 0;
    virtual bool readTemperature(float& temperature) = 0;
    virtual float calculateAltitude(float pressure, float seaLevelPressure = 1013.25f) = 0;
    virtual bool isHealthy() = 0;
};

#endif // IBADOMETER_H

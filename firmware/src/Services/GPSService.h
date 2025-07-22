#pragma once

#include "HAL/IGPS.h"
#include "Data/Types.h"

class GPSService
{
public:
    GPSService(IGPS& gps);

    void update();

    GPSData getGPSData() const;
    FlightState getFlightState() const;
    float getSpeed() const;
    float getHeading() const;

private:
    IGPS& gps;
    GPSData gpsData;
    FlightState flightState;

    void detectFlightState();
};

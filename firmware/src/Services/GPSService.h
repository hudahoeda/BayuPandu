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
    // True when the module delivered data on the last update, fix or not.
    bool isResponding() const { return responding; }

private:
    IGPS& gps;
    GPSData gpsData;
    FlightState flightState;
    bool responding = false;

    void detectFlightState();
};

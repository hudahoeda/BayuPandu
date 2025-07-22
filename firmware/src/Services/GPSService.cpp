#include "GPSService.h"
#include <Arduino.h>

GPSService::GPSService(IGPS& gps)
    : gps(gps),
      flightState(FlightState::GROUND)
{
}

void GPSService::update()
{
    gps.update();
    gpsData = gps.getCurrentPosition();
    detectFlightState();
}

GPSData GPSService::getGPSData() const
{
    return gpsData;
}

FlightState GPSService::getFlightState() const
{
    return flightState;
}

float GPSService::getSpeed() const
{
    return gpsData.speed;
}

float GPSService::getHeading() const
{
    return gpsData.heading;
}

void GPSService::detectFlightState()
{
    // Simple flight state detection logic
    switch (flightState)
    {
        case FlightState::GROUND:
            if (gpsData.speed > 5.0f) // 5 m/s ~ 18 km/h
            {
                flightState = FlightState::TAKEOFF;
            }
            break;
        case FlightState::TAKEOFF:
            if (gpsData.speed > 8.0f) // 8 m/s ~ 28.8 km/h
            {
                flightState = FlightState::FLYING;
            }
            break;
        case FlightState::FLYING:
            if (gpsData.speed < 3.0f) // 3 m/s ~ 10.8 km/h
            {
                flightState = FlightState::LANDED;
            }
            break;
        case FlightState::LANDED:
            // Stay in landed state
            break;
    }
}

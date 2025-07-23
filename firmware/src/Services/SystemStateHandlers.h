#pragma once

#include "Data/Types.h"
#include "HAL/IArduino.h"

// Abstract base class for system state handlers
class SystemStateHandler {
public:
    virtual ~SystemStateHandler() = default;
    
    // Called when entering this state
    virtual void onEnter() {}
    
    // Called when exiting this state
    virtual void onExit() {}
    
    // Called periodically while in this state
    // Returns the next state to transition to (or current state to stay)
    virtual SystemState update() = 0;
    
    // Called to handle events specific to this state
    virtual void handleEvent(const char* event) { (void)event; }

protected:
    SystemStateHandler(IArduino& arduino) : arduino(arduino) {}
    
    IArduino& arduino;
};

// Forward declarations
class FlightManager;

// Concrete state handlers
class InitializingStateHandler : public SystemStateHandler {
public:
    InitializingStateHandler(FlightManager& manager, IArduino& arduino);
    SystemState update() override;
    
private:
    FlightManager& flightManager;
};

class ReadyStateHandler : public SystemStateHandler {
public:
    ReadyStateHandler(FlightManager& manager, IArduino& arduino);
    SystemState update() override;
    
private:
    FlightManager& flightManager;
};

class FlightActiveStateHandler : public SystemStateHandler {
public:
    FlightActiveStateHandler(FlightManager& manager, IArduino& arduino);
    SystemState update() override;
    void onEnter() override;
    void onExit() override;
    
private:
    FlightManager& flightManager;
    uint32_t landingStartTime;
    static const uint32_t LANDING_CONFIRMATION_TIME = 30000; // 30 seconds
};

class LowPowerStateHandler : public SystemStateHandler {
public:
    LowPowerStateHandler(FlightManager& manager, IArduino& arduino);
    SystemState update() override;
    void onEnter() override;
    
private:
    FlightManager& flightManager;
    uint32_t lastUpdateTime;
    static const uint32_t LOW_POWER_UPDATE_INTERVAL = 5000; // 5 seconds
};

class ErrorStateHandler : public SystemStateHandler {
public:
    ErrorStateHandler(FlightManager& manager, IArduino& arduino);
    SystemState update() override;
    void onEnter() override;
    
private:
    FlightManager& flightManager;
    uint32_t lastRecoveryAttempt;
    static const uint32_t RECOVERY_ATTEMPT_INTERVAL = 10000; // 10 seconds
};

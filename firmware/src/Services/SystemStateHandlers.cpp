#include "SystemStateHandlers.h"
#include "Services/FlightManager.h"

// InitializingStateHandler implementation
InitializingStateHandler::InitializingStateHandler(FlightManager& manager, IArduino& arduino) :
    SystemStateHandler(arduino),
    flightManager(manager)
{
}

SystemState InitializingStateHandler::update() {
    // Try to initialize sensors
    if (flightManager.initializeSensors()) {
        flightManager.getHealthMonitor().clearError();
        return SystemState::READY;
    }
    
    // Stay in initializing state if sensors fail
    return SystemState::INITIALIZING;
}

// ReadyStateHandler implementation
ReadyStateHandler::ReadyStateHandler(FlightManager& manager, IArduino& arduino) :
    SystemStateHandler(arduino),
    flightManager(manager)
{
}

SystemState ReadyStateHandler::update() {
    // Check for low power condition
    if (flightManager.getPowerService().isCriticalBattery()) {
        return SystemState::LOW_POWER;
    }
    
    // Check if flight has started
    FlightState flightState = flightManager.getGPSService().getFlightState();
    if (flightState == FlightState::FLYING || flightState == FlightState::TAKEOFF) {
        return SystemState::FLIGHT_ACTIVE;
    }
    
    return SystemState::READY;
}

// FlightActiveStateHandler implementation
FlightActiveStateHandler::FlightActiveStateHandler(FlightManager& manager, IArduino& arduino) :
    SystemStateHandler(arduino),
    flightManager(manager),
    landingStartTime(0)
{
}

void FlightActiveStateHandler::onEnter() {
    landingStartTime = 0; // Reset landing timer
}

void FlightActiveStateHandler::onExit() {
    landingStartTime = 0; // Reset landing timer
}

SystemState FlightActiveStateHandler::update() {
    // Check for low power condition (critical in flight)
    if (flightManager.getPowerService().isCriticalBattery()) {
        return SystemState::LOW_POWER;
    }
    
    // Check if flight has ended
    FlightState currentFlightState = flightManager.getGPSService().getFlightState();
    if (currentFlightState == FlightState::GROUND || currentFlightState == FlightState::LANDED) {
        // Wait a bit before transitioning to avoid false positives
        if (landingStartTime == 0) {
            landingStartTime = arduino.millis();
        } else if (arduino.millis() - landingStartTime > LANDING_CONFIRMATION_TIME) {
            return SystemState::READY;
        }
    } else {
        // Reset landing timer if still flying
        landingStartTime = 0;
    }
    
    return SystemState::FLIGHT_ACTIVE;
}

// LowPowerStateHandler implementation
LowPowerStateHandler::LowPowerStateHandler(FlightManager& manager, IArduino& arduino) :
    SystemStateHandler(arduino),
    flightManager(manager),
    lastUpdateTime(0)
{
}

void LowPowerStateHandler::onEnter() {
    lastUpdateTime = arduino.millis();
}

SystemState LowPowerStateHandler::update() {
    // Reduce update frequency in low power mode
    uint32_t currentTime = arduino.millis();
    if (currentTime - lastUpdateTime < LOW_POWER_UPDATE_INTERVAL) {
        return SystemState::LOW_POWER; // Stay in low power mode
    }
    lastUpdateTime = currentTime;
    
    // Check if power situation has improved
    PowerService& powerService = flightManager.getPowerService();
    if (!powerService.isCriticalBattery() && powerService.getBatteryVoltage() > 3.5f) {
        FlightState flightState = flightManager.getGPSService().getFlightState();
        if (flightState == FlightState::FLYING) {
            return SystemState::FLIGHT_ACTIVE;
        } else {
            return SystemState::READY;
        }
    }
    
    return SystemState::LOW_POWER;
}

// ErrorStateHandler implementation
ErrorStateHandler::ErrorStateHandler(FlightManager& manager, IArduino& arduino) :
    SystemStateHandler(arduino),
    flightManager(manager),
    lastRecoveryAttempt(0)
{
}

void ErrorStateHandler::onEnter() {
    lastRecoveryAttempt = arduino.millis();
}

SystemState ErrorStateHandler::update() {
    // Try to recover from error state
    uint32_t currentTime = arduino.millis();
    if (currentTime - lastRecoveryAttempt > RECOVERY_ATTEMPT_INTERVAL) {
        lastRecoveryAttempt = currentTime;
        
        if (flightManager.initializeSensors()) {
            flightManager.getHealthMonitor().clearError();
            return SystemState::READY;
        }
    }
    
    return SystemState::ERROR;
}

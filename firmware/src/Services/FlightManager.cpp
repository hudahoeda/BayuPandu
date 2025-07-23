#include "FlightManager.h"
#include "UI/UserInterface.h"
#include <memory>

FlightManager::FlightManager(
    VariometerService& variometerService,
    GPSService& gpsService,
    IMUService& imuService,
    PowerService& powerService,
    ConfigService& configService,
    IStorage& storage,
    IArduino& arduino
) : 
    variometerService(variometerService),
    gpsService(gpsService),
    imuService(imuService),
    powerService(powerService),
    configService(configService),
    storage(storage),
    arduino(arduino),
    userInterface(nullptr),
    dataFusion(variometerService, gpsService, imuService, arduino),
    healthMonitor(dataFusion, arduino),
    flightLogger(storage),
    currentState(SystemState::INITIALIZING)
{
    createStateHandlers();
}

void FlightManager::setUserInterface(UserInterface* ui) {
    userInterface = ui;
}

bool FlightManager::initialize() {
    // Load configuration first
    configService.loadConfig();
    
    // Set initial state
    setState(SystemState::INITIALIZING);
    
    return true;
}

void FlightManager::update() {
    // Update all services
    variometerService.update();
    gpsService.update();
    imuService.update();
    powerService.update();
    
    // Update modular components
    dataFusion.fuseData();
    healthMonitor.update();
    flightLogger.update(getFusedFlightData(), getFlightState());
    
    // Update UI if available
    if (userInterface) {
        userInterface->update();
    }
    
    // Check for health-based state transitions
    if (healthMonitor.hasError() && currentState != SystemState::ERROR) {
        setState(SystemState::ERROR);
        return;
    }
    
    // Update current state handler and check for state transitions
    if (currentStateHandler) {
        SystemState nextState = currentStateHandler->update();
        if (nextState != currentState) {
            setState(nextState);
        }
    }
}

void FlightManager::shutdown() {
    // Save any pending configuration
    configService.saveConfig();
    
    // Set system to safe state
    setState(SystemState::ERROR);
}

SystemState FlightManager::getSystemState() const {
    return currentState;
}

FlightState FlightManager::getFlightState() const {
    return gpsService.getFlightState();
}

FlightData FlightManager::getFusedFlightData() const {
    return dataFusion.getFusedFlightData();
}

bool FlightManager::isDataValid() const {
    return dataFusion.isDataValid() && healthMonitor.areAllSensorsHealthy();
}

bool FlightManager::areAllSensorsHealthy() const {
    return healthMonitor.areAllSensorsHealthy();
}

const char* FlightManager::getLastError() const {
    return healthMonitor.getLastError();
}

bool FlightManager::initializeSensors() {
    // This would normally initialize hardware sensors
    // For now, assume they're initialized through the services
    return true;
}

// Private methods implementation

void FlightManager::createStateHandlers() {
    initializingHandler.reset(new InitializingStateHandler(*this, arduino));
    readyHandler.reset(new ReadyStateHandler(*this, arduino));
    flightActiveHandler.reset(new FlightActiveStateHandler(*this, arduino));
    lowPowerHandler.reset(new LowPowerStateHandler(*this, arduino));
    errorHandler.reset(new ErrorStateHandler(*this, arduino));
    
    // Set initial state handler
    currentStateHandler = getStateHandler(currentState);
}

void FlightManager::setState(SystemState newState) {
    if (newState != currentState) {
        // Exit current state
        if (currentStateHandler) {
            currentStateHandler->onExit();
        }
        
        currentState = newState;
        currentStateHandler = getStateHandler(newState);
        
        // Enter new state
        if (currentStateHandler) {
            currentStateHandler->onEnter();
        }
    }
}

SystemStateHandler* FlightManager::getStateHandler(SystemState state) {
    switch (state) {
        case SystemState::INITIALIZING: return initializingHandler.get();
        case SystemState::READY: return readyHandler.get();
        case SystemState::FLIGHT_ACTIVE: return flightActiveHandler.get();
        case SystemState::LOW_POWER: return lowPowerHandler.get();
        case SystemState::ERROR: return errorHandler.get();
    }
    return nullptr;
}

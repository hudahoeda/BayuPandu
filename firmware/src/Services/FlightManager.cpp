#include "FlightManager.h"
#include "UI/UserInterface.h"
#include "Data/Types.h" // For BatteryInfo
#include <memory>
#include <string.h> // For strcmp

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
    simulationService(arduino), // Initialize SimulationService
    currentState(SystemState::INITIALIZING),
    simulationActiveFlag(false) // Initialize simulation flag
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
    if (simulationActiveFlag) {
        simulationService.update(); // Update simulation state

        // Create a FlightData object from simulated data
        FlightData simulatedData;
        simulatedData.altitude = simulationService.getBarometricAltitude();
        simulatedData.verticalSpeed = simulationService.getVerticalSpeed();
        simulatedData.gpsData = simulationService.getGPSData();
        simulatedData.attitude = simulationService.getAttitudeData();
        simulatedData.timestamp = arduino.millis(); // Use current simulation time
        simulatedData.isValid = simulationService.isActive(); // Data is valid if simulation is active
        
        dataFusion.setFusedFlightData(simulatedData);
        
        // HealthMonitor and FlightLogger should ideally be adapted for simulation,
        // but for now, they will operate on the simulated fused data.
        healthMonitor.update(); 
        flightLogger.update(getFusedFlightData(), getFlightState());
    } else {
        // Update all services from real sensors
        variometerService.update();
        gpsService.update();
        imuService.update();
        powerService.update();
        
        // Update modular components
        dataFusion.fuseData(); // Uses data from real services
        healthMonitor.update();
        flightLogger.update(getFusedFlightData(), getFlightState());
    }
    
    // Update UI if available
    if (userInterface) {
        userInterface->update();
        updateAlerts(); // Update alerts after UI update
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

// Removed initializeSensors() as it's no longer a member function

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

void FlightManager::enableSimulation(const std::string& igcFilePath) {
    if (simulationService.initialize(igcFilePath)) {
        simulationActiveFlag = true;
        // Optionally, reset system state or inform UI
        if (userInterface) {
            userInterface->showAlert("Simulation Enabled", 2000);
        }
    } else {
        if (userInterface) {
            userInterface->showAlert("Simulation Failed to Load", 3000);
        }
    }
}

void FlightManager::disableSimulation() {
    simulationActiveFlag = false;
    // Optionally, reset system state or inform UI
    if (userInterface) {
        userInterface->clearAlert(); // Clear any simulation-related alerts
        userInterface->showAlert("Simulation Disabled", 2000);
    }
}

bool FlightManager::isSimulationActive() const {
    return simulationActiveFlag;
}

void FlightManager::updateAlerts() {
    if (!userInterface) return;

    const char* currentAlert = nullptr;
    if (healthMonitor.hasError()) {
        currentAlert = healthMonitor.getLastError();
    } else if (simulationActiveFlag) {
        // In simulation mode, alerts might be based on simulated data or specific simulation events
        // For now, we'll keep it simple and just show a "SIMULATION ACTIVE" message
        currentAlert = "SIMULATION ACTIVE";
    }
    else { // Only check real sensor issues if not in simulation
        BatteryInfo battery = powerService.getBatteryInfo();
        if (powerService.isCriticalBattery()) {
            currentAlert = "CRITICAL BATTERY!";
        } else if (powerService.isLowBattery()) {
            currentAlert = "LOW BATTERY!";
        } else if (!gpsService.getGPSData().hasValidFix) {
            currentAlert = "NO GPS FIX";
        }
    }

    // If there's a new alert or the current alert has changed
    if (currentAlert && (lastAlertMessage == nullptr || strcmp(lastAlertMessage, currentAlert) != 0)) {
        userInterface->showAlert(currentAlert);
        lastAlertMessage = currentAlert;
    } 
    // If there was an alert but now there isn't, clear it
    else if (lastAlertMessage && currentAlert == nullptr) {
        userInterface->clearAlert();
        lastAlertMessage = nullptr;
    }
}

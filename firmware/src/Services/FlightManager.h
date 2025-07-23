#pragma once

#include "Data/Types.h"
#include "Services/VariometerService.h"
#include "Services/GPSService.h"
#include "Services/IMUService.h"
#include "Services/PowerService.h"
#include "Services/ConfigService.h"
#include "Services/DataFusionManager.h"
#include "Services/HealthMonitor.h"
#include "Services/FlightLogger.h"
#include "Services/SystemStateHandlers.h"
#include "Services/SimulationService.h" // Include SimulationService
#include "HAL/IArduino.h"
#include <memory>

// Forward declaration to avoid circular dependency
class UserInterface;


// FlightManager orchestrates system initialization, updates, and data fusion
class FlightManager {
public:
    FlightManager(
        VariometerService& variometerService,
        GPSService& gpsService,
        IMUService& imuService,
        PowerService& powerService,
        ConfigService& configService,
        IStorage& storage,
        IArduino& arduino
    );
    
    // UI Integration
    void setUserInterface(UserInterface* ui);
    
    // System lifecycle
    bool initialize();
    void update();
    void shutdown();
    
    // System state management
    SystemState getSystemState() const;
    FlightState getFlightState() const;
    
    // Data access
    FlightData getFusedFlightData() const;
    bool isDataValid() const;
    
    // Health monitoring
    bool areAllSensorsHealthy() const;
    const char* getLastError() const;
    
    // Internal access for state handlers
    PowerService& getPowerService() { return powerService; }
    GPSService& getGPSService() { return gpsService; }
    ConfigService& getConfigService() { return configService; }
    HealthMonitor& getHealthMonitor() { return healthMonitor; }
    // Simulation management
    void enableSimulation(const std::string& igcFilePath);
    void disableSimulation();
    bool isSimulationActive() const;

private:
    // Service dependencies
    VariometerService& variometerService;
    GPSService& gpsService;
    IMUService& imuService;
    PowerService& powerService;
    ConfigService& configService;
    IStorage& storage;
    IArduino& arduino;
    
    // UI Reference
    UserInterface* userInterface;
    
    // Modular components
    DataFusionManager dataFusion;
    HealthMonitor healthMonitor;
    FlightLogger flightLogger;
    SimulationService simulationService; // Add SimulationService instance
    
    // State management
    SystemState currentState;
    SystemStateHandler* currentStateHandler;
    std::unique_ptr<SystemStateHandler> initializingHandler;
    std::unique_ptr<SystemStateHandler> readyHandler;
    std::unique_ptr<SystemStateHandler> flightActiveHandler;
    std::unique_ptr<SystemStateHandler> lowPowerHandler;
    std::unique_ptr<SystemStateHandler> errorHandler;
    
    // Alert state
    const char* lastAlertMessage = nullptr;

    // Simulation state
    bool simulationActiveFlag;

    // Helper methods
    void createStateHandlers();
    void setState(SystemState newState);
    SystemStateHandler* getStateHandler(SystemState state);
    void updateAlerts();
};

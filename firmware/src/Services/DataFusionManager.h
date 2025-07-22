#pragma once

#include "Data/Types.h"
#include "Services/VariometerService.h"
#include "Services/GPSService.h"
#include "Services/IMUService.h"
#include "HAL/IArduino.h"

// Handles sensor data fusion and validation
class DataFusionManager {
public:
    DataFusionManager(
        VariometerService& variometerService,
        GPSService& gpsService,
        IMUService& imuService,
        IArduino& arduino
    );
    
    // Perform data fusion from all sensors
    void fuseData();
    
    // Get the fused flight data
    FlightData getFusedFlightData() const;
    
    // Check if the fused data is valid
    bool isDataValid() const;

private:
    VariometerService& variometerService;
    GPSService& gpsService;
    IMUService& imuService;
    IArduino& arduino;
    
    FlightData fusedFlightData;
    
    // Individual fusion methods
    void fuseAltitudeData();
    void fusePositionData();
    void fuseAttitudeData();
    void validateFusedData();
};

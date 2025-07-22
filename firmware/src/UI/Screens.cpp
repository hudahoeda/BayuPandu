#include "Screens.h"
#include "UserInterface.h"
#include <stdio.h>
#include <math.h>

// Helper functions (previously in UserInterface.cpp)
namespace UIHelper {
    void formatAltitude(float altitude, char* buffer, size_t size) {
        snprintf(buffer, size, "%.0fm", altitude);
    }

    void formatVerticalSpeed(float vspeed, char* buffer, size_t size) {
        snprintf(buffer, size, "%.1f", vspeed);
    }

    void formatGPSCoordinate(double coordinate, bool isLatitude, char* buffer, size_t size) {
        char hemisphere = isLatitude ? (coordinate >= 0 ? 'N' : 'S') : (coordinate >= 0 ? 'E' : 'W');
        double absCoord = fabs(coordinate);
        int degrees = (int)absCoord;
        double minutes = (absCoord - degrees) * 60.0;
        snprintf(buffer, size, "%d°%.3f'%c", degrees, minutes, hemisphere);
    }

    void formatTime(uint32_t seconds, char* buffer, size_t size) {
        unsigned int hours = seconds / 3600;
        unsigned int minutes = (seconds % 3600) / 60;
        unsigned int secs = seconds % 60;
        snprintf(buffer, size, "%02u:%02u:%02u", hours, minutes, secs);
    }

    void drawHeader(IDisplay& display, FlightManager& flightManager, IArduino& /*arduino*/) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        
        // Flight state indicator
        display.setCursor(5, 5);
        FlightState state = flightManager.getFlightState();
        switch (state) {
            case FlightState::GROUND: display.print("GND"); break;
            case FlightState::TAKEOFF: display.print("T/O"); break;
            case FlightState::FLYING: display.print("FLY"); break;
            case FlightState::LANDED: display.print("LND"); break;
        }
        
        // GPS indicator
        display.setCursor(60, 5);
        FlightData data = flightManager.getFusedFlightData();
        if (data.gpsData.hasValidFix) {
            display.print("GPS:OK");
        } else {
            display.print("GPS:--");
        }
        
        // Battery indicator
        display.setCursor(100, 5);
        display.print("BAT:OK");
        
        // Draw header separator line
        display.drawLine(0, 15, 128, 15, 1);
    }

    void drawFooter(IDisplay& display, IArduino& arduino) {
        // Draw footer separator line
        display.drawLine(0, 148, 128, 148, 1);
        
        display.setTextSize(1);
        display.setCursor(5, 152);
        
        // Show current time
        char timeBuffer[16];
        uint32_t seconds = arduino.millis() / 1000;
        formatTime(seconds, timeBuffer, sizeof(timeBuffer));
        display.print(timeBuffer);
    }
}

// MainFlightScreen implementation
void MainFlightScreen::update() {
    FlightData flightData = flightManager.getFusedFlightData();
    UIHelper::drawHeader(display, flightManager, arduino);
    
    // Main altitude display (large font)
    char altBuffer[16];
    UIHelper::formatAltitude(flightData.altitude, altBuffer, sizeof(altBuffer));
    display.setTextSize(3);
    display.setCursor(10, 30);
    display.print("ALT");
    display.setCursor(10, 60);
    display.print(altBuffer);
    
    // Vertical speed (medium font)
    char vspeedBuffer[16];
    UIHelper::formatVerticalSpeed(flightData.verticalSpeed, vspeedBuffer, sizeof(vspeedBuffer));
    display.setTextSize(2);
    display.setCursor(10, 100);
    display.print("VS: ");
    display.print(vspeedBuffer);
    
    // GPS info (small font)
    if (flightData.gpsData.hasValidFix) {
        display.setTextSize(1);
        display.setCursor(10, 130);
        display.print("SPD: ");
        display.print((int)flightData.gpsData.speed);
        display.print(" km/h");
        
        display.setCursor(10, 145);
        display.print("SAT: ");
        display.print(flightData.gpsData.satellites);
    } else {
        display.setTextSize(1);
        display.setCursor(10, 130);
        display.print("GPS: NO FIX");
    }
    
    UIHelper::drawFooter(display, arduino);
}

void MainFlightScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    switch (buttonId) {
        case 0: // UP button
            if (action == ButtonAction::SHORT_PRESS) {
                ui.setScreen(DisplayScreen::NAVIGATION);
            }
            break;
        case 1: // DOWN button
            if (action == ButtonAction::SHORT_PRESS) {
                ui.setScreen(DisplayScreen::STATUS);
            }
            break;
        case 2: // LEFT button
            if (action == ButtonAction::LONG_PRESS) {
                ui.setScreen(DisplayScreen::SETTINGS);
            }
            break;
        case 3: // RIGHT button / SELECT
            // Could toggle units or show more info
            break;
    }
}

// NavigationScreen implementation
void NavigationScreen::update() {
    FlightData flightData = flightManager.getFusedFlightData();
    GPSData gpsData = flightData.gpsData;
    
    UIHelper::drawHeader(display, flightManager, arduino);
    
    display.setTextSize(1);
    display.setCursor(5, 30);
    display.print("NAVIGATION");
    
    if (gpsData.hasValidFix) {
        // Coordinates
        char latBuffer[16], lonBuffer[16];
        UIHelper::formatGPSCoordinate(gpsData.latitude, true, latBuffer, sizeof(latBuffer));
        UIHelper::formatGPSCoordinate(gpsData.longitude, false, lonBuffer, sizeof(lonBuffer));
        
        display.setCursor(5, 50);
        display.print("LAT: ");
        display.print(latBuffer);
        
        display.setCursor(5, 65);
        display.print("LON: ");
        display.print(lonBuffer);
        
        // Speed and heading
        display.setCursor(5, 85);
        display.print("SPEED: ");
        display.print((int)gpsData.speed);
        display.print(" km/h");
        
        display.setCursor(5, 100);
        display.print("HDG: ");
        display.print((int)gpsData.heading);
        display.print("°");
        
        // GPS quality
        display.setCursor(5, 120);
        display.print("HDOP: ");
        display.print(gpsData.hdop, 1);
        display.print(" SAT: ");
        display.print(gpsData.satellites);
    } else {
        display.setCursor(5, 50);
        display.print("WAITING FOR GPS FIX...");
    }
    
    UIHelper::drawFooter(display, arduino);
}

void NavigationScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    switch (buttonId) {
        case 0: // UP button
            ui.setScreen(DisplayScreen::MAIN_FLIGHT);
            break;
        case 1: // DOWN button
            ui.setScreen(DisplayScreen::STATUS);
            break;
        case 2: // LEFT button
            ui.setScreen(DisplayScreen::SETTINGS);
            break;
    }
}

// SettingsScreen implementation
void SettingsScreen::update() {
    SystemConfig& config = configService.getConfig();
    
    UIHelper::drawHeader(display, flightManager, arduino);
    
    display.setTextSize(1);
    display.setCursor(5, 30);
    display.print("SETTINGS");
    
    display.setCursor(5, 50);
    display.print("Brightness: ");
    display.print((int)config.brightness);
    display.print("%");
    
    display.setCursor(5, 65);
    display.print("Volume: ");
    display.print((int)config.audioVolume);
    display.print("%");
    
    display.setCursor(5, 80);
    display.print("Lift Thres: ");
    display.print(config.liftThreshold, 1);
    display.print(" m/s");
    
    display.setCursor(5, 95);
    display.print("Sink Thres: ");
    display.print(config.sinkThreshold, 1);
    display.print(" m/s");
    
    display.setCursor(5, 110);
    display.print("QNH: ");
    display.print(config.qnhPressure, 1);
    display.print(" hPa");
    
    display.setCursor(5, 130);
    display.print("Log Interval: ");
    display.print((int)config.loggingInterval);
    display.print("s");
    
    UIHelper::drawFooter(display, arduino);
}

void SettingsScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    
    switch (buttonId) {
        case 0: // UP button - navigate settings or increase values
            break;
        case 1: // DOWN button - navigate settings or decrease values
            break;
        case 2: // LEFT button - back to main
            configService.saveConfig();
            ui.setScreen(DisplayScreen::MAIN_FLIGHT);
            break;
        case 3: // RIGHT button - confirm/edit
            break;
    }
}

// StatusScreen implementation
void StatusScreen::update() {
    SystemState systemState = flightManager.getSystemState();
    FlightState flightState = flightManager.getFlightState();
    
    UIHelper::drawHeader(display, flightManager, arduino);
    
    display.setTextSize(1);
    display.setCursor(5, 30);
    display.print("SYSTEM STATUS");
    
    // System state
    display.setCursor(5, 50);
    display.print("System: ");
    switch (systemState) {
        case SystemState::INITIALIZING: display.print("INIT"); break;
        case SystemState::READY: display.print("READY"); break;
        case SystemState::FLIGHT_ACTIVE: display.print("FLIGHT"); break;
        case SystemState::LOW_POWER: display.print("LOW PWR"); break;
        case SystemState::ERROR: display.print("ERROR"); break;
    }
    
    // Flight state
    display.setCursor(5, 65);
    display.print("Flight: ");
    switch (flightState) {
        case FlightState::GROUND: display.print("GROUND"); break;
        case FlightState::TAKEOFF: display.print("TAKEOFF"); break;
        case FlightState::FLYING: display.print("FLYING"); break;
        case FlightState::LANDED: display.print("LANDED"); break;
    }
    
    // Sensor health
    display.setCursor(5, 80);
    display.print("Sensors: ");
    display.print(flightManager.areAllSensorsHealthy() ? "OK" : "ERROR");
    
    // Data validity
    display.setCursor(5, 95);
    display.print("Data: ");
    display.print(flightManager.isDataValid() ? "VALID" : "INVALID");
    
    // Error message if any
    const char* error = flightManager.getLastError();
    if (error[0] != '\0') {
        display.setCursor(5, 110);
        display.print("Error: ");
        display.print(error);
    }
    
    UIHelper::drawFooter(display, arduino);
}

void StatusScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    switch (buttonId) {
        case 0: // UP button
            ui.setScreen(DisplayScreen::NAVIGATION);
            break;
        case 1: // DOWN button
            ui.setScreen(DisplayScreen::MAIN_FLIGHT);
            break;
        case 2: // LEFT button
            ui.setScreen(DisplayScreen::SETTINGS);
            break;
    }
}

// ErrorScreen implementation
void ErrorScreen::update() {
    UIHelper::drawHeader(display, flightManager, arduino);
    
    display.setTextSize(2);
    display.setCursor(20, 50);
    display.print("ERROR");
    
    display.setTextSize(1);
    display.setCursor(5, 80);
    const char* error = flightManager.getLastError();
    if (error[0] != '\0') {
        display.print(error);
    } else {
        display.print("System Error");
    }
    
    display.setCursor(5, 100);
    display.print("Press any button");
    display.setCursor(5, 115);
    display.print("for status");
    
    UIHelper::drawFooter(display, arduino);
}

void ErrorScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    (void)buttonId;
    // Any button press in error screen goes to status
    ui.setScreen(DisplayScreen::STATUS);
}

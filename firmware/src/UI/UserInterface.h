#pragma once

#include "Data/Types.h"
#include "Services/FlightManager.h"
#include "Services/ConfigService.h"
#include "HAL/IDisplay.h"
#include "HAL/IArduino.h"
#include "UI/InputManager.h"
#include "UI/Screen.h"
#include <memory>

enum class DisplayScreen {
    MAIN_FLIGHT,
    NAVIGATION,
    SETTINGS,
    STATUS,
    ERROR
};

class UserInterface {
public:
    UserInterface(
        IDisplay& display,
        FlightManager& flightManager,
        ConfigService& configService,
        IArduino& arduino
    );

    void initialize();
    void update();

    // Screen management
    void setScreen(DisplayScreen screen);
    DisplayScreen getCurrentScreen() const;

    // Button handling
    void handleButtonAction(ButtonAction action, uint8_t buttonId);
    
    // Alert management
    void showAlert(const char* message, uint16_t durationMs = 3000);
    void clearAlert();

private:
    // Dependencies
    IDisplay& display;
    FlightManager& flightManager;
    ConfigService& configService;
    IArduino& arduino;

    // Input management
    InputManager inputManager;

    // Screen management
    DisplayScreen currentScreenType;
    std::unique_ptr<Screen> currentScreen;

    // UI state
    uint32_t lastUpdate;
    uint32_t screenTimeout;
    bool displayActive;

    // Alert system
    char alertMessage[64];
    uint32_t alertStartTime;
    uint16_t alertDuration;
    bool alertActive;

    // Screen refresh rates (ms)
    static const uint32_t MAIN_REFRESH_RATE = 500;    // 2 Hz for main flight screen
    static const uint32_t NAV_REFRESH_RATE = 1000;    // 1 Hz for navigation
    static const uint32_t STATUS_REFRESH_RATE = 2000; // 0.5 Hz for status
    static const uint32_t SETTINGS_REFRESH_RATE = 100; // 10 Hz for responsive settings

    // Helper methods
    void activateDisplay();
    void handleScreenTimeout();
    void displayAlert();
};

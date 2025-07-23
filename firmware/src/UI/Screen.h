#pragma once

#include "HAL/IDisplay.h"
#include "Services/FlightManager.h"
#include "Services/ConfigService.h"
#include "HAL/IArduino.h"
#include "UI/InputManager.h"

class UserInterface; // Forward declaration

// Abstract base class for all UI screens
class Screen {
public:
    virtual ~Screen() = default;

    // Called when the screen becomes active
    virtual void onEnter() {}

    // Called when the screen is no longer active
    virtual void onExit() {}

    // Called periodically to update and draw the screen
    virtual void update() = 0;

    // Called to handle button input
    virtual void handleInput(ButtonAction action, uint8_t buttonId) = 0;

    // Called to handle serial input for simulation
    virtual void handleSerialInput(char serialChar) {}

    Screen(UserInterface& ui, IDisplay& display, FlightManager& flightManager, ConfigService& configService, IArduino& arduino)
        : ui(ui), display(display), flightManager(flightManager), configService(configService), arduino(arduino) {}

protected:
    UserInterface& ui;
    IDisplay& display;
    FlightManager& flightManager;
    ConfigService& configService;
    IArduino& arduino;
};

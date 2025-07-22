#include "UserInterface.h"
#include "UI/Screens.h"
#include <string.h>
#include <memory>

UserInterface::UserInterface(
    IDisplay& display,
    FlightManager& flightManager,
    ConfigService& configService,
    IArduino& arduino
) :
    display(display),
    flightManager(flightManager),
    configService(configService),
    arduino(arduino),
    inputManager(arduino),
    currentScreenType(DisplayScreen::MAIN_FLIGHT),
    lastUpdate(0),
    screenTimeout(0),
    displayActive(true),
    alertMessage{0},
    alertStartTime(0),
    alertDuration(0),
    alertActive(false)
{
}

void UserInterface::initialize() {
    display.initialize();
    
    // Set initial screen timeout from config
    SystemConfig& config = configService.getConfig();
    screenTimeout = config.screenTimeout * 1000; // Convert to milliseconds
    
    // Initialize display
    display.clear();
    display.setBrightness(config.brightness);
    
    // Set up input manager callback
    inputManager.setButtonActionCallback([this](ButtonAction action, uint8_t buttonId) {
        this->handleButtonAction(action, buttonId);
    });
    
    // Set initial screen
    setScreen(DisplayScreen::MAIN_FLIGHT);
    
    lastUpdate = arduino.millis();
    displayActive = true;
}

void UserInterface::update() {
    uint32_t currentTime = arduino.millis();
    
    // Update input manager
    inputManager.update();
    
    // Handle screen timeout
    if (displayActive && screenTimeout > 0) {
        if (currentTime - lastUpdate > screenTimeout) {
            displayActive = false;
            display.setBrightness(0);
        }
    }
    
    // Handle alert timeout
    if (alertActive && currentTime - alertStartTime > alertDuration) {
        clearAlert();
    }
    
    // Update current screen if display is active
    if (displayActive && currentScreen) {
        display.clear();
        currentScreen->update();
        
        // Display alert overlay if active
        if (alertActive) {
            displayAlert();
        }
    }
    
    lastUpdate = currentTime;
}

void UserInterface::setScreen(DisplayScreen screen) {
    if (screen != currentScreenType || !currentScreen) {
        // Exit current screen
        if (currentScreen) {
            currentScreen->onExit();
        }
        
        currentScreenType = screen;
        
        // This is where we create a new screen instance
        switch (screen) {
            case DisplayScreen::MAIN_FLIGHT:
                currentScreen = std::make_unique<MainFlightScreen>(*this, display, flightManager, configService, arduino);
                break;
            case DisplayScreen::NAVIGATION:
                currentScreen = std::make_unique<NavigationScreen>(*this, display, flightManager, configService, arduino);
                break;
            case DisplayScreen::SETTINGS:
                currentScreen = std::make_unique<SettingsScreen>(*this, display, flightManager, configService, arduino);
                break;
            case DisplayScreen::STATUS:
                currentScreen = std::make_unique<StatusScreen>(*this, display, flightManager, configService, arduino);
                break;
            case DisplayScreen::ERROR:
                currentScreen = std::make_unique<ErrorScreen>(*this, display, flightManager, configService, arduino);
                break;
            default:
                currentScreen = nullptr;
                break;
        }
        
        // Enter new screen
        if (currentScreen) {
            currentScreen->onEnter();
        }
        
        // Force immediate update
        lastUpdate = arduino.millis();
    }
}

DisplayScreen UserInterface::getCurrentScreen() const {
    return currentScreenType;
}

void UserInterface::handleButtonAction(ButtonAction action, uint8_t buttonId) {
    activateDisplay(); // Wake up display on any button press
    
    if (action == ButtonAction::NONE) return;
    
    // Handle global actions first
    if (alertActive && action == ButtonAction::SHORT_PRESS) {
        clearAlert();
        return;
    }
    
    // Pass input to current screen
    if (currentScreen) {
        currentScreen->handleInput(action, buttonId);
    }
}

void UserInterface::showAlert(const char* message, uint16_t durationMs) {
    if (message && strlen(message) < sizeof(alertMessage)) {
        strcpy(alertMessage, message);
        alertStartTime = arduino.millis();
        alertDuration = durationMs;
        alertActive = true;
        activateDisplay(); // Make sure display is on for alert
    }
}

void UserInterface::clearAlert() {
    alertActive = false;
    alertMessage[0] = '\0';
}

void UserInterface::activateDisplay() {
    if (!displayActive) {
        SystemConfig& config = configService.getConfig();
        display.setBrightness(config.brightness);
        displayActive = true;
    }
    lastUpdate = arduino.millis(); // Reset timeout
}

void UserInterface::displayAlert() {
    // Draw alert box (overlay on current screen)
    display.drawRect(10, 40, 108, 40, 1); // Border
    display.fillRect(11, 41, 106, 38, 0);  // Clear background
    
    display.setTextSize(1);
    display.setCursor(15, 50);
    display.print("ALERT");
    
    display.setCursor(15, 65);
    display.print(alertMessage);
}

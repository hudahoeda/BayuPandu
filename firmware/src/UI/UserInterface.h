#pragma once

#include "Data/Types.h"
#include "Services/FlightManager.h"
#include "Services/ConfigService.h"
#include "HAL/IArduino.h"
#include "UI/LVGLInit.h"
#include "lvgl.h"
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
    FlightManager& flightManager;
    ConfigService& configService;
    IArduino& arduino;

    // LVGL screen objects
    lv_obj_t* mainFlightScreen = nullptr;
    lv_obj_t* navigationScreen = nullptr;
    lv_obj_t* settingsScreen = nullptr;
    lv_obj_t* statusScreen = nullptr;
    lv_obj_t* errorScreen = nullptr;
    
    // Alert management
    lv_obj_t* alertModal = nullptr;
    lv_timer_t* alertTimer = nullptr;

    // Screen management
    DisplayScreen currentScreenType;
    lv_obj_t* currentScreenObj = nullptr;

    // UI state
    uint32_t lastUpdate;
    bool displayActive;

    // Screen refresh rates (ms)
    static const uint32_t MAIN_REFRESH_RATE = 500;    // 2 Hz for main flight screen
    static const uint32_t NAV_REFRESH_RATE = 1000;    // 1 Hz for navigation
    static const uint32_t STATUS_REFRESH_RATE = 2000; // 0.5 Hz for status
    static const uint32_t SETTINGS_REFRESH_RATE = 100; // 10 Hz for responsive settings

    // Screen creation methods
    lv_obj_t* createMainFlightScreen();
    lv_obj_t* createNavigationScreen();
    lv_obj_t* createSettingsScreen();
    lv_obj_t* createStatusScreen();
    lv_obj_t* createErrorScreen();
    
    // Screen update methods
    void updateMainFlightScreen();
    void updateNavigationScreen();
    void updateSettingsScreen();
    void updateStatusScreen();
    void updateErrorScreen();
    
    // Alert helpers
    void createAlert(const char* message, uint16_t durationMs);
    static void alertTimerCallback(lv_timer_t* timer);
    
    // Button event handlers
    static void buttonEventHandler(lv_event_t* e);
    
    // Helper methods
    void loadCurrentScreen();
    bool shouldUpdateScreen();
};

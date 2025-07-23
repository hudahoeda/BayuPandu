#pragma once

#include "UI/Screen.h"
#include "lvgl.h"

class MainFlightScreen : public Screen {
public:
    using Screen::Screen;
    void onEnter() override;
    void onExit() override;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
    void handleSerialInput(char serialChar) override;
    
private:
    void createWidgets();
    void updateFlightData();
    
    void createSimulationElements();
    void updateSimulationStatus();

    lv_obj_t* screen = nullptr;
    lv_obj_t* altitudeLabel = nullptr;
    lv_obj_t* verticalSpeedBar = nullptr;
    lv_obj_t* speedLabel = nullptr;
    lv_obj_t* satelliteLabel = nullptr;
    lv_obj_t* flightStateLabel = nullptr;
    lv_obj_t* gpsStatusLabel = nullptr;
    lv_obj_t* batteryBar = nullptr;
    lv_obj_t* timeLabel = nullptr;

    // Simulation UI elements
    lv_obj_t* simBanner = nullptr;
    lv_obj_t* simFilenameLabel = nullptr;
    lv_obj_t* simSpeedLabel = nullptr;
    lv_obj_t* simProgressBar = nullptr;
};

class NavigationScreen : public Screen {
public:
    using Screen::Screen;
    void onEnter() override;
    void onExit() override;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
    
private:
    void createWidgets();
    void updateNavigationData();
    
    lv_obj_t* screen = nullptr;
    lv_obj_t* latitudeLabel = nullptr;
    lv_obj_t* longitudeLabel = nullptr;
    lv_obj_t* speedLabel = nullptr;
    lv_obj_t* headingLabel = nullptr;
    lv_obj_t* hdopLabel = nullptr;
    lv_obj_t* satelliteLabel = nullptr;
    lv_obj_t* compassArc = nullptr;
};

class SettingsScreen : public Screen {
public:
    using Screen::Screen;
    void onEnter() override;
    void onExit() override;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
    
private:
    void createWidgets();
    void updateSettings();
    
    lv_obj_t* screen = nullptr;
    lv_obj_t* brightnessSlider = nullptr;
    lv_obj_t* volumeSlider = nullptr;
    lv_obj_t* liftThresholdSlider = nullptr;
    lv_obj_t* sinkThresholdSlider = nullptr;
    lv_obj_t* qnhLabel = nullptr;
    lv_obj_t* loggingIntervalLabel = nullptr;
    lv_obj_t* settingLabels[6] = {nullptr};
};

class StatusScreen : public Screen {
public:
    using Screen::Screen;
    void onEnter() override;
    void onExit() override;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
    
private:
    void createWidgets();
    void updateSystemStatus();
    
    lv_obj_t* screen = nullptr;
    lv_obj_t* systemStateLabel = nullptr;
    lv_obj_t* flightStateLabel = nullptr;
    lv_obj_t* sensorHealthLabel = nullptr;
    lv_obj_t* dataValidLabel = nullptr;
    lv_obj_t* errorLabel = nullptr;
    lv_obj_t* uptimeLabel = nullptr;
};

class ErrorScreen : public Screen {
public:
    using Screen::Screen;
    void onEnter() override;
    void onExit() override;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
    
private:
    void createWidgets();
    
    lv_obj_t* screen = nullptr;
    lv_obj_t* errorTitleLabel = nullptr;
    lv_obj_t* errorMessageLabel = nullptr;
    lv_obj_t* instructionLabel = nullptr;
};

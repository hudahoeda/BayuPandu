#pragma once

#include "UI/Screen.h"

class MainFlightScreen : public Screen {
public:
    using Screen::Screen;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
};

class NavigationScreen : public Screen {
public:
    using Screen::Screen;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
};

class SettingsScreen : public Screen {
public:
    using Screen::Screen;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
};

class StatusScreen : public Screen {
public:
    using Screen::Screen;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
};

class ErrorScreen : public Screen {
public:
    using Screen::Screen;
    void update() override;
    void handleInput(ButtonAction action, uint8_t buttonId) override;
};

#include "config.h"

#ifdef ARDUINO
#include <Arduino.h>
#include "HAL/ArduinoImpl.h"
#else
#include <ArduinoFake.h>
#include "HAL/ArduinoFakeImpl.h"
#endif

#include "HAL/IArduino.h"
#include "Services/VariometerService.h"
#include "Services/GPSService.h"
#include "Services/IMUService.h"
#include "Services/PowerService.h"
#include "Services/ConfigService.h"
#include "Services/FlightManager.h"
#include "UI/UserInterface.h"
#include "HAL/StorageImpl.h"
#include "UI/LVGLInit.h"

// Global Arduino abstraction
IArduino* arduino_impl = nullptr;

// HAL implementations
#include "HAL/BarometerImpl.h"
#include "HAL/AudioImpl.h"
#include "HAL/GPSImpl.h"
#include "HAL/IMUImpl.h"
#include "HAL/PowerImpl.h"

BarometerImpl barometer;
AudioImpl audio;
GPSImpl gps;
IMUImpl imu;
PowerImpl power;
StorageImpl storage;

// Services
VariometerService* variometerService = nullptr;
GPSService* gpsService = nullptr;
IMUService* imuService = nullptr;
PowerService* powerService = nullptr;
ConfigService* configService = nullptr;

// Flight Manager and UI
FlightManager* flightManager = nullptr;
UserInterface* userInterface = nullptr;

void setup() {
#ifdef ARDUINO
  arduino_impl = new ArduinoImpl();
#else
  arduino_impl = new ArduinoFakeImpl();
#endif

  // Instantiate services
  configService = new ConfigService(storage);
  variometerService = new VariometerService(barometer, audio, *arduino_impl);
  gpsService = new GPSService(gps);
  imuService = new IMUService(imu);
  powerService = new PowerService(power, audio, *configService, *arduino_impl);

  // Initialize LVGL
  LVGLInit::initialize(*arduino_impl);

  // Instantiate Flight Manager
  flightManager = new FlightManager(
    *variometerService,
    *gpsService,
    *imuService,
    *powerService,
    *configService,
    storage,
    *arduino_impl
  );

  // Instantiate User Interface
  userInterface = new UserInterface(
    *flightManager,
    *configService,
    *arduino_impl
  );

  // Connect UI to FlightManager
  flightManager->setUserInterface(userInterface);
  
  // Connect UI to input system
  LVGLInit::setUserInterface(userInterface);

  // Initialize systems
  flightManager->initialize();
  userInterface->initialize();
}

void loop() {
  // Handle LVGL tasks (this will also handle InputManager updates)
  LVGLInit::handler();
  
  // Update flight manager (which will update UI)
  flightManager->update();
}

#ifndef ARDUINO
// For native testing, provide a main function
int main() {
  setup();
  for (int i = 0; i < 5; i++) {  // Run loop a few times for testing
    loop();
  }
  delete userInterface;
  delete flightManager;
  delete configService;
  delete powerService;
  delete imuService;
  delete gpsService;
  delete variometerService;
  delete arduino_impl;
  return 0;
}
#endif

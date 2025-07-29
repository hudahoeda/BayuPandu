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
#include "UI/InputManager.h"
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
InputManager* inputManager = nullptr;

void setup() {
#ifdef ARDUINO
  Serial.begin(115200);
  Serial.println("BayuPandu Variometer Starting...");
  Serial.println("DEBUG: Before arduino_impl instantiation");
  arduino_impl = new ArduinoImpl();
  Serial.println("DEBUG: After arduino_impl instantiation");
#else
  arduino_impl = new ArduinoFakeImpl();
#endif

  // Instantiate services
  Serial.println("Initializing services...");
  Serial.println("DEBUG: Before ConfigService instantiation");
  configService = new ConfigService(storage);
  Serial.println("DEBUG: After ConfigService instantiation");
  Serial.println("DEBUG: Before VariometerService instantiation");
  variometerService = new VariometerService(barometer, audio, *arduino_impl);
  Serial.println("DEBUG: After VariometerService instantiation");
  Serial.println("DEBUG: Before GPSService instantiation");
  gpsService = new GPSService(gps);
  Serial.println("DEBUG: After GPSService instantiation");
  Serial.println("DEBUG: Before IMUService instantiation");
  imuService = new IMUService(imu);
  Serial.println("DEBUG: After IMUService instantiation");
  Serial.println("DEBUG: Before PowerService instantiation");
  powerService = new PowerService(power, audio, *configService, *arduino_impl);
  Serial.println("DEBUG: After PowerService instantiation");

  // Initialize LVGL
  Serial.println("Initializing LVGL...");
  Serial.println("DEBUG: Before LVGLInit::initialize");
  LVGLInit::initialize(*arduino_impl);
  Serial.println("DEBUG: After LVGLInit::initialize");

  // Instantiate Flight Manager
  Serial.println("Initializing Flight Manager...");
  Serial.println("DEBUG: Before FlightManager instantiation");
  flightManager = new FlightManager(
    *variometerService,
    *gpsService,
    *imuService,
    *powerService,
    *configService,
    storage,
    *arduino_impl
  );
  Serial.println("DEBUG: After FlightManager instantiation");

  // Instantiate User Interface
  Serial.println("Initializing User Interface...");
  Serial.println("DEBUG: Before UserInterface instantiation");
  userInterface = new UserInterface(
    *flightManager,
    *configService,
    *arduino_impl,
    *LVGLInit::inputManager
  );
  Serial.println("DEBUG: After UserInterface instantiation");

  // Connect UI to FlightManager
  Serial.println("DEBUG: Before flightManager->setUserInterface");
  flightManager->setUserInterface(userInterface);
  Serial.println("DEBUG: After flightManager->setUserInterface");
  
  // Connect UI to input system
  Serial.println("DEBUG: Before LVGLInit::setUserInterface");
  LVGLInit::setUserInterface(userInterface);
  Serial.println("DEBUG: After LVGLInit::setUserInterface");

  // Initialize systems
  Serial.println("Initializing systems...");
  Serial.println("DEBUG: Before flightManager->initialize");
  flightManager->initialize();
  Serial.println("DEBUG: After flightManager->initialize");
  Serial.println("DEBUG: Before userInterface->initialize");
  userInterface->initialize();
  Serial.println("DEBUG: After userInterface->initialize");
  Serial.println("Initialization complete!");
}

void loop() {
  // Handle LVGL tasks (this will also handle InputManager updates)
  LVGLInit::handler();
  
  // Update flight manager (which will update UI)
  flightManager->update();
}

#if !defined(UNIT_TEST)
// For native testing, provide a main function
int main() {
  setup();
  for (int i = 0; i < 5; i++) {  // Run loop a few times for testing
    loop();
  }
  delete userInterface;
  delete inputManager;
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

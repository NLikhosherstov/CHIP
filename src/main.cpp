#include <Arduino.h>
#include "automation/AutomationController.h"
#include "display/DisplayManager.h"
#include "input/InputController.h"
#include "sensors/Max31855Sensor.h"
#include "sensors/Dht22Sensor.h"
#include "system/SerialHandler.h"
#include "system/SystemState.h"
#include "system/ConfigManager.h"
#include "pins.h"
#include "wiring_constants.h"

static SystemState            g_state;
static ConfigManager          g_cfg;
static SerialHandler          g_serial(g_state, g_cfg);
static Max31855Sensor         g_heat_sensor(g_state);
static Dht22Sensor            g_room_sensor(g_state);
static DisplayManager*        g_display     = nullptr;
static AutomationController*  g_automation  = nullptr;
static InputController*       g_input       = nullptr;

void setup() {
  Serial.begin(115200);
  pinMode(PC13, OUTPUT); // Встроенный светодиод

  // Инициализация пинов в режим выходов и принудительное их выключение
  pinMode(pin::MOTOR,   OUTPUT); digitalWrite(pin::MOTOR,   LOW);
  pinMode(pin::IGNITOR, OUTPUT); digitalWrite(pin::IGNITOR, LOW);
  pinMode(pin::PUMP,    OUTPUT); digitalWrite(pin::PUMP,    LOW);

  // while (!Serial) {
  //   digitalWrite(PC13, !digitalRead(PC13));
  //   delay(100);
  // }

  const bool config_loaded = g_cfg.load();
  (void)config_loaded;

  g_heat_sensor.begin();
  // g_heat_sensor.setTracingEnabled(true);

  g_room_sensor.begin();
  
  g_display = new DisplayManager();
  g_display->begin(g_state, g_cfg);

  g_automation = new AutomationController(g_state, g_cfg);
  g_automation->begin();

  g_input = new InputController(*g_display, *g_automation, g_state, g_cfg);
  g_input->begin();
}

// Порядок в loop: сенсоры → автоматика → ввод → дисплей (оператор может перебить automation).

void loop() {
  g_serial.tick();
  g_heat_sensor.tick();
  g_room_sensor.tick();

  if (g_automation) {
    g_automation->tick(millis());
  }
  if (g_input) {
    g_input->tick();
  }
  if (g_display) {
    g_display->tick(g_state, g_cfg);
  }
}

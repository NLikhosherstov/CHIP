#include <Arduino.h>
#include "automation/AutomationController.h"
#include "display/DisplayManager.h"
#include "input/InputController.h"
#include "sensors/Max31855Sensor.h"
#include "sensors/Dht22Sensor.h"
#include "system/SystemState.h"
#include "system/ConfigManager.h"
#include "pins.h"
#include "wiring_constants.h"

void monitorSerialConnectionData();
void rebootToTinyUF2();

bool wasSerialConnected = false;
bool hasLoggedStatus = false;

static SystemState            g_state;
static ConfigManager          g_cfg;
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
  monitorSerialConnectionData();
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

void monitorSerialConnectionData() {
  if(Serial.available() > 0){
    String cmd = Serial.readString();
    if(cmd.indexOf("BOOT") >= 0){
      digitalWrite(PC13, HIGH);
      rebootToTinyUF2();
    }
  }

  bool isSerialConnected = (bool)Serial; 
  if(isSerialConnected && !wasSerialConnected){
    if(!hasLoggedStatus){
      //Configuration status
      Serial.println(F("\n===================================="));
      Serial.println(F("Config loading status:"));
      
      if(g_cfg.isLoaded()){
        Serial.println(F("  - SUCCESS: User config loaded from Flash/EEPROM."));
        if(g_cfg.getConfig().btnPower == 0){
          Serial.println(F("  - KBD: Calibration required!"));
        }
      }else{
        Serial.println(F("  - WARNING: Flash was empty/corrupted. Default settings applied."));
        Serial.println(F("  - KBD: Calibration required!"));
      }
      Serial.println(F("===================================="));
      
      hasLoggedStatus = true; // Блок на повторный вывод лога в порт
    }
  }else if(!isSerialConnected && wasSerialConnected){
    hasLoggedStatus = false; 
  }

  wasSerialConnected = isSerialConnected;
}

void rebootToTinyUF2() {  
  constexpr uint32_t DBL_TAP_MAGIC = 0xf01669efUL;
  constexpr uint32_t DBL_TAP_ADDR  = 0x20000000 + 64 * 1024 - 4;

  // Запись признака о необходимости загрузиться в режиме загрузчика
  __disable_irq(); // Отключаем прерывания
  volatile uint32_t *boot_mode = (volatile uint32_t *)DBL_TAP_ADDR;
  *boot_mode = DBL_TAP_MAGIC;

  asm volatile("dsb");
  NVIC_SystemReset();
}
#include <Arduino.h>
#include "display/DisplayManager.h"
#include "system/SystemState.h"
#include "system/ConfigManager.h"
#include "pins.h"

bool wasSerialConnected = false;
bool hasLoggedStatus = false;
void monitorSerialConnectionData();

static SystemState     g_state;
static ConfigManager   g_cfg;
static DisplayManager* g_display = nullptr;

void setup() {
  Serial.begin(115200);
  pinMode(PC13, OUTPUT); // Встроенный светодиод

  // Инициализация пинов в режим выходов и принудительное их выключение
  pinMode(pin::MOTOR,   OUTPUT); digitalWrite(pin::MOTOR,   LOW);
  pinMode(pin::IGNITOR, OUTPUT); digitalWrite(pin::IGNITOR, LOW);
  pinMode(pin::PUMP,    OUTPUT); digitalWrite(pin::PUMP,    LOW);

  const bool config_loaded = g_cfg.load();
  
  g_display = new DisplayManager();
  g_display->begin(g_state, g_cfg);
}

void loop() {
  monitorSerialConnectionData();

  if(g_display){
    g_display->tick(g_state, g_cfg);
  }
}

void monitorSerialConnectionData() {
  bool isSerialConnected = (bool)Serial; 
  if(isSerialConnected && !wasSerialConnected){
    if(!hasLoggedStatus){
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
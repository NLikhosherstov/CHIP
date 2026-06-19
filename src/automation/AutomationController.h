#pragma once

#include <stdint.h>

#include "input/Event.h"
#include "automation/IgnitorController.h"
#include "automation/MotorController.h"
#include "automation/PumpController.h"
#include "system/SystemState.h"

class ConfigManager;

// Автономная логика печки: FSM, исполнители, обработка SystemRequest из UI.
// Не знает о кнопках и дисплее — только публичный API команд.
class AutomationController {
public:
  AutomationController(SystemState& state, ConfigManager& config);

  void begin();
  void tick(uint32_t now_ms);
  void event(Event& e);

  void requestStart();
  void requestStop();
  void emergencyStop();
  void enterAuto();
  void enterManual();
  void setMotorStep(int8_t step_0_to_4);
  void setPumpStep(int8_t step_0_to_4);
  void setIgnitorEnabled(bool on);

private:
  void processRequests();

  SystemState& m_state;
  ConfigManager& m_config;
  PumpController m_pump;
  MotorController m_motor;
  IgnitorController m_ignitor;
  uint8_t m_lastPumpStep = 1;
};

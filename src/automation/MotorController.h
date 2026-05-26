#pragma once

#include <stdint.h>

class SystemState;
class ConfigManager;

// Управление мотором (ступени 1..4, PWM из конфига).
class MotorController {
public:
  explicit MotorController(SystemState& state, const ConfigManager& config);

  void begin();
  void setEnabled(bool enabled);
  void setStep(uint8_t step_1_to_4);

private:
  SystemState& m_state;
  const ConfigManager& m_config;
};

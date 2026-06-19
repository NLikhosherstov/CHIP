#pragma once

#include <stdint.h>

class SystemState;
class ConfigManager;

// Управление мотором (ступени 0..4, PWM из конфига).
class MotorController {
public:
  explicit MotorController(SystemState& state, const ConfigManager& config);

  void begin();
  void setStep(int8_t step_0_to_4);

private:
  SystemState& m_state;
  const ConfigManager& m_config;
};

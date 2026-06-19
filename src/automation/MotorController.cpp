#include "automation/MotorController.h"

#include <Arduino.h>

#include "pins.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

uint8_t pwmForStep(const ConfigManager::Config& conf, uint8_t step_0_to_4) {
  switch (step_0_to_4) {
    case 0:
      return 0;
    case 1:
      return conf.motor_PWM_1;
    case 2:
      return conf.motor_PWM_2;
    case 3:
      return conf.motor_PWM_3;
    case 4:
    default:
      return conf.motor_PWM_4;
  }
}

}  // namespace

MotorController::MotorController(SystemState& state, const ConfigManager& config)
    : m_state(state), m_config(config) {}

void MotorController::begin() {
  setStep(0);
}

void MotorController::setStep(int8_t step_0_to_4) {
  const uint8_t step = (step_0_to_4 < 0) ? 0 : (step_0_to_4 > 4) ? 4 : step_0_to_4;
  const bool enabled = (step != 0);
  const uint8_t pwm = pwmForStep(m_config.getConfig(), step);
  const uint16_t duty = enabled ? static_cast<uint16_t>(pwm) * 4 : 0;
  digitalWrite(pin::MOTOR, enabled ? HIGH : LOW);
  m_state.setMotorState(enabled, step, duty, enabled ? 20000 : 0);
}

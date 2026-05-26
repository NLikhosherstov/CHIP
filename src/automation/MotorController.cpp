#include "automation/MotorController.h"

#include <Arduino.h>

#include "pins.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

uint8_t pwmForStep(const ConfigManager::Config& conf, uint8_t step_1_to_4) {
  switch (step_1_to_4) {
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
  setStep(1);
  setEnabled(false);
}

void MotorController::setEnabled(bool enabled) {
  digitalWrite(pin::MOTOR, enabled ? HIGH : LOW);
  const auto motor = m_state.getMotorState();
  const uint8_t step = static_cast<uint8_t>(motor.speed_index + 1);
  const uint8_t pwm = pwmForStep(m_config.getConfig(), step);
  const uint16_t duty = enabled ? static_cast<uint16_t>(pwm) * 4 : 0;
  m_state.setMotorState(enabled, motor.speed_index, duty, enabled ? 20000 : 0);
}

void MotorController::setStep(uint8_t step_1_to_4) {
  const uint8_t step = (step_1_to_4 < 1) ? 1 : (step_1_to_4 > 4) ? 4 : step_1_to_4;
  const uint8_t index = static_cast<uint8_t>(step - 1);
  const bool enabled = m_state.getMotorState().enabled;
  const uint8_t pwm = pwmForStep(m_config.getConfig(), step);
  const uint16_t duty = enabled ? static_cast<uint16_t>(pwm) * 4 : 0;
  m_state.setMotorState(enabled, index, duty, enabled ? 20000 : 0);
}

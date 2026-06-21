#include "automation/MotorController.h"

#include <Arduino.h>
#include <HardwareTimer.h>

#include "pins.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

uint16_t pwmForStep(const ConfigManager::Config& conf, uint8_t step) {
  switch (step) {
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

MotorController::~MotorController() {
  delete m_timer;
  m_timer = nullptr;
}

void MotorController::begin() {
  if (m_timer == nullptr) {
    m_timer = new HardwareTimer(TIM1);
  }
  m_timer->setPWM(kPwmChannel, pin::MOTOR, kPwmFrequencyHz, 0);
  m_timer->pause();

  m_current_duty = 0;
  m_target_duty = 0;
  m_target_step = 0;
  m_ramping = false;
  setStep(0);
}

void MotorController::setStep(int8_t step) {
  const uint8_t clampStep = (step < 0) ? 0 : (step > 4) ? 4 : step;
  m_target_step = clampStep;

  if (clampStep == 0) {
    m_target_duty = 0;
    if (m_current_duty == 0) {
      m_ramping = false;
      stopPwmHardware();
      publishState();
      return;
    }
    m_ramping = true;
    publishState();
    return;
  }

  m_target_duty = pwmForStep(m_config.getConfig(), clampStep);

  if (m_current_duty == 0) {
    // Старт с нуля: kickstart 35%, затем рампа к целевому duty.
    m_current_duty = kKickstartPermille;
    applyDuty(m_current_duty);
    m_ramping = (m_current_duty != m_target_duty);
    m_last_ramp_ms = millis();
  } else {
    m_ramping = (m_current_duty != m_target_duty);
  }

  publishState();
}

void MotorController::tick(uint32_t now_ms) {
  if (!m_ramping) {
    return;
  }
  if (now_ms - m_last_ramp_ms < kRampIntervalMs) {
    return;
  }
  m_last_ramp_ms = now_ms;

  if (m_current_duty < m_target_duty) {
    const uint32_t next = static_cast<uint32_t>(m_current_duty) + kRampStepPermille;
    m_current_duty = static_cast<uint16_t>((next > m_target_duty) ? m_target_duty : next);
  } else if (m_current_duty > m_target_duty) {
    const uint16_t step = (m_current_duty > kRampStepPermille) ? kRampStepPermille : m_current_duty;
    const uint16_t next = m_current_duty - step;
    m_current_duty = (next < m_target_duty) ? m_target_duty : next;
  }

  if (m_current_duty > 0) {
    applyDuty(m_current_duty);
  } else {
    stopPwmHardware();
  }

  publishState();

  if (m_current_duty == m_target_duty) {
    m_ramping = false;
    if (m_current_duty == 0) {
      stopPwmHardware();
    }
  }
}

void MotorController::stopImmediate() {
  m_target_step = 0;
  m_target_duty = 0;
  m_current_duty = 0;
  m_ramping = false;
  stopPwmHardware();
  publishState();
}

void MotorController::applyDuty(uint16_t permille) {
  // Установка скважности ШИМ TIM1 на PA8 (permille → проценты).
  if (m_timer == nullptr) {
    return;
  }

  const uint16_t clamped = (permille > kDutyPermilleMax) ? kDutyPermilleMax : permille;
  const uint8_t percent = static_cast<uint8_t>((clamped + 5u) / 10u);

  if (!m_timer->isRunning()) {
    m_timer->setPWM(kPwmChannel, pin::MOTOR, kPwmFrequencyHz, percent);
  } else {
    m_timer->setCaptureCompare(kPwmChannel, percent, PERCENT_COMPARE_FORMAT);
  }
}

void MotorController::publishState() {
  const bool enabled = (m_target_step != 0) || (m_current_duty > 0);
  const uint32_t frequency = enabled ? kPwmFrequencyHz : 0;
  m_state.setMotorState(enabled, m_target_step, m_current_duty, frequency);
}

void MotorController::stopPwmHardware() {
  // Остановка ШИМ и принудительный LOW на выходе.
  if (m_timer != nullptr) {
    m_timer->pause();
    m_timer->setCaptureCompare(kPwmChannel, 0, PERCENT_COMPARE_FORMAT);
  }
  digitalWrite(pin::MOTOR, LOW);
}

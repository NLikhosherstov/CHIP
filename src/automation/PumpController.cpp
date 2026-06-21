#include "automation/PumpController.h"

#include <Arduino.h>
#include <HardwareTimer.h>

#include "pins.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

constexpr uint16_t PUMP_PULSE_ON_MS = 45;
constexpr uint16_t MIN_CYCLE_MS = PUMP_PULSE_ON_MS + 1;
constexpr uint16_t MAX_CYCLE_MS = 1000;

uint16_t baseCycleMsForStep(const ConfigManager::Config& conf, uint8_t step) {
  switch (step) {
    case 1:
      return conf.pump_pulse_1;
    case 2:
      return conf.pump_pulse_2;
    case 3:
      return conf.pump_pulse_3;
    case 4:
      return conf.pump_pulse_4;
    default:
      return 0;
  }
}

uint16_t effectiveCycleMs(const ConfigManager::Config& conf, uint8_t step) {
  if (step == 0) {
    return 0;
  }

  const uint16_t base = baseCycleMsForStep(conf, step);
  int32_t safe_correction = conf.fuel_correction;
  if (safe_correction < -50) safe_correction = -50; 

  int32_t corrected = (static_cast<int32_t>(base) * 100) / (100 + safe_correction);
  if (corrected < MIN_CYCLE_MS) corrected = MIN_CYCLE_MS;
  if (corrected > MAX_CYCLE_MS) corrected = MAX_CYCLE_MS;

  return static_cast<uint16_t>(corrected);
}

}  // namespace

PumpController* PumpController::s_instance = nullptr;

PumpController::PumpController(SystemState& state, const ConfigManager& config)
    : m_state(state), m_config(config) {
  s_instance = this;
}

PumpController::~PumpController() {
  if (s_instance == this) {
    s_instance = nullptr;
  }
  delete m_timer;
  m_timer = nullptr;
}

void PumpController::begin() {
  pinMode(pin::PUMP, OUTPUT);
  digitalWrite(pin::PUMP, LOW);

  if (m_timer == nullptr) {
    m_timer = new HardwareTimer(TIM4);
    m_timer->setMode(1, TIMER_OUTPUT_DISABLED);
    m_timer->attachInterrupt(timerIrqThunk);
  }
  m_timer->pause();

  setStep(0);
}

void PumpController::setStep(int8_t step) {
  const uint8_t clampStep = (step < 0) ? 0 : (step > 4) ? 4 : step;
  const bool enabled = (clampStep != 0);
  m_cycle_ms = enabled ? effectiveCycleMs(m_config.getConfig(), clampStep) : 0;
  m_pause_ms = enabled ? static_cast<uint16_t>(m_cycle_ms - PUMP_PULSE_ON_MS) : 0;

  m_state.setPumpState(enabled, clampStep, m_cycle_ms);

  if (!enabled) {
    stopSequencer();
  } else if (!m_running) {
    startSequencer();
  }
}

void PumpController::refreshTiming() {
  const auto pump = m_state.getPumpState();
  if (pump.speed_index == 0) {
    return;
  }

  m_cycle_ms = effectiveCycleMs(m_config.getConfig(), pump.speed_index);
  m_pause_ms = static_cast<uint16_t>(m_cycle_ms - PUMP_PULSE_ON_MS);
  m_state.setPumpState(true, pump.speed_index, m_cycle_ms);
}

void PumpController::startSequencer() {
  m_running = true;
  m_phase = Phase::On;
  digitalWrite(pin::PUMP, HIGH);
  armTimerMs(PUMP_PULSE_ON_MS);
}

void PumpController::stopSequencer() {
  m_running = false;
  m_phase = Phase::Stopped;
  if (m_timer != nullptr) {
    m_timer->pause();
  }
  digitalWrite(pin::PUMP, LOW);
}

void PumpController::armTimerMs(uint16_t ms) {
  if (m_timer == nullptr) {
    return;
  }
  m_timer->pause();
  m_timer->setOverflow(static_cast<uint32_t>(ms) * 1000U, MICROSEC_FORMAT);
  m_timer->setCount(0);
  m_timer->refresh();
  m_timer->resume();
}

void PumpController::timerIrqThunk() {
  if (s_instance != nullptr) {
    s_instance->onTimerIrq();
  }
}

void PumpController::onTimerIrq() {
  if (!m_running) {
    return;
  }

  if (m_phase == Phase::On) {
    digitalWrite(pin::PUMP, LOW);
    m_phase = Phase::Off;
    armTimerMs(m_pause_ms);
  } else {
    digitalWrite(pin::PUMP, HIGH);
    m_phase = Phase::On;
    armTimerMs(PUMP_PULSE_ON_MS);
  }
}

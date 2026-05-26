#include "automation/PumpController.h"

#include <Arduino.h>

#include "pins.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

uint16_t pulseHzFromIndex(const ConfigManager::Config& conf, uint8_t index) {
  switch (index) {
    case 0:
      return conf.pump_pulse_1;
    case 1:
      return conf.pump_pulse_2;
    case 2:
      return conf.pump_pulse_3;
    case 3:
    default:
      return conf.pump_pulse_4;
  }
}

}  // namespace

PumpController::PumpController(SystemState& state, const ConfigManager& config)
    : m_state(state), m_config(config) {}

void PumpController::begin() {
  setEnabled(false);
}

void PumpController::setEnabled(bool enabled) {
  digitalWrite(pin::PUMP, enabled ? HIGH : LOW);
  const uint8_t idx = m_state.getPumpState().speed_index;
  m_state.setPumpState(enabled, idx, enabled ? pulseHzFromIndex(m_config.getConfig(), idx) : 0);
}

void PumpController::setSpeedIndex(uint8_t index_0_to_3) {
  const uint8_t idx = (index_0_to_3 > 3) ? 3 : index_0_to_3;
  const bool enabled = m_state.getPumpState().enabled;
  m_state.setPumpState(enabled, idx, enabled ? pulseHzFromIndex(m_config.getConfig(), idx) : 0);
}

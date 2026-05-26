#include "automation/IgnitorController.h"

#include <Arduino.h>

#include "pins.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

IgnitorController::IgnitorController(SystemState& state, const ConfigManager& config)
    : m_state(state), m_config(config) {}

void IgnitorController::begin() {
  setEnabled(false);
}

void IgnitorController::setEnabled(bool enabled) {
  digitalWrite(pin::IGNITOR, enabled ? HIGH : LOW);
  const auto& conf = m_config.getConfig();
  const uint32_t deadline =
      enabled ? (millis() + static_cast<uint32_t>(conf.ignitor_timeout_s) * 1000UL) : 0;
  m_state.setIgnitorState(enabled, enabled ? 100 : 0, deadline);
}

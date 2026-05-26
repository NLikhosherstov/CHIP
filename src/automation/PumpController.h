#pragma once

#include <stdint.h>

class SystemState;
class ConfigManager;

// Управление топливным насосом (GPIO + снимок в SystemState).
class PumpController {
public:
  explicit PumpController(SystemState& state, const ConfigManager& config);

  void begin();
  void setEnabled(bool enabled);
  void setSpeedIndex(uint8_t index_0_to_3);

private:
  SystemState& m_state;
  const ConfigManager& m_config;
};

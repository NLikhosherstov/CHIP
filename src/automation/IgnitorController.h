#pragma once

#include <stdint.h>

class SystemState;
class ConfigManager;

// Управление свечой накала (GPIO + таймаут из конфига).
class IgnitorController {
public:
  explicit IgnitorController(SystemState& state, const ConfigManager& config);

  void begin();
  void setEnabled(bool enabled);

private:
  SystemState& m_state;
  const ConfigManager& m_config;
};

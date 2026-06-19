#pragma once

#include <stdint.h>

class HardwareTimer;
class SystemState;
class ConfigManager;

// Управление топливным насосом ТН-10: импульсы 45 мс через аппаратный таймер.
class PumpController {
public:
  explicit PumpController(SystemState& state, const ConfigManager& config);
  ~PumpController();

  void begin();
  void setStep(int8_t step_0_to_4);

private:
  enum class Phase : uint8_t { Stopped, On, Off };

  void startSequencer();
  void stopSequencer();
  void armTimerMs(uint16_t ms);
  void onTimerIrq();
  static void timerIrqThunk();

  SystemState& m_state;
  const ConfigManager& m_config;
  HardwareTimer* m_timer = nullptr;
  volatile bool m_running = false;
  volatile Phase m_phase = Phase::Stopped;
  uint16_t m_pause_ms = 0;
  uint16_t m_cycle_ms = 0;

  static PumpController* s_instance;
};

#pragma once

#include <stdint.h>

class HardwareTimer;
class SystemState;
class ConfigManager;

// Управление мотором (ступени 0..4, ШИМ с плавной рампой).
class MotorController {
public:
  explicit MotorController(SystemState& state, const ConfigManager& config);
  ~MotorController();

  void begin();
  void setStep(int8_t step);
  // Плавное изменение duty к целевому значению; вызывается из loop через AutomationController.
  void tick(uint32_t now_ms);
  // Мгновенное отключение без рампы (аварийный stop).
  void stopImmediate();

private:
  static constexpr uint32_t kPwmFrequencyHz = 20000;
  static constexpr uint16_t kDutyPermilleMax = 1000;
  static constexpr uint16_t kKickstartPermille = 350;
  static constexpr uint16_t kRampStepPermille = 2;
  static constexpr uint16_t kRampIntervalMs = 10;
  static constexpr uint8_t kPwmChannel = 1;

  void applyDuty(uint16_t permille);
  void publishState();
  void stopPwmHardware();

  SystemState& m_state;
  const ConfigManager& m_config;
  HardwareTimer* m_timer = nullptr;

  uint16_t m_current_duty = 0;
  uint16_t m_target_duty = 0;
  uint8_t m_target_step = 0;
  bool m_ramping = false;
  uint32_t m_last_ramp_ms = 0;
};

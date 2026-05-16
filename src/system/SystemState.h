#pragma once

#include "system/SystemState.h"
#include <Arduino.h>

// #include <stdint.h>

class Max31855Sensor;
class Dht22Sensor;
class IgnitorController;
class PumpController;
class MotorController;
class AutomationController;

class SystemState {
public:
  enum class AutomationState : uint8_t {
    STATE_IDLE = 0,
    STATE_AUTO_START,
    STATE_AUTO_TSTAT,
    STATE_MANUAL,
    STATE_STOP
  };

  struct HeatExchangerState {
    float temperature_c = 0.0F;
    bool valid = false;
    uint32_t updated_at_ms = 0;
  };

  struct RoomClimateState {
    float temperature_c = 0.0F;
    float humidity_percent = 0.0F;
    bool valid = false;
    uint32_t updated_at_ms = 0;
  };

  struct IgnitorState {
    bool enabled = false;
    uint8_t pwm_percent = 0;
    uint32_t timeout_deadline_ms = 0;
  };

  struct PumpState {
    bool enabled = false;
    uint8_t speed_index = 0;
    uint16_t pulse_hz = 0;
  };

  struct MotorState {
    bool enabled = false;
    uint8_t speed_index = 0;
    uint16_t pwm_duty_permille = 0;
    uint32_t pwm_frequency_hz = 0;
  };

  SystemState() = default;

  HeatExchangerState getHeatExchangerState() const;
  RoomClimateState getRoomClimateState() const;
  IgnitorState getIgnitorState() const;
  PumpState getPumpState() const;
  MotorState getMotorState() const;
  AutomationState getAutomationState() const;

private:
  friend class Max31855Sensor;
  friend class Dht22Sensor;
  friend class IgnitorController;
  friend class PumpController;
  friend class MotorController;
  friend class AutomationController;

  void setHeatExchangerState(float temperature_c, bool valid, uint32_t updated_at_ms);
  void setRoomClimateState(float temperature_c, float humidity_percent, bool valid, uint32_t updated_at_ms);
  void setIgnitorState(bool enabled, uint8_t pwm_percent, uint32_t timeout_deadline_ms);
  void setPumpState(bool enabled, uint8_t speed_index, uint16_t pulse_hz);
  void setMotorState(bool enabled, uint8_t speed_index, uint16_t pwm_duty_permille, uint32_t pwm_frequency_hz);
  void setAutomationState(AutomationState state);

private:
  HeatExchangerState m_heat_exchanger_state;
  RoomClimateState m_room_climate_state;
  IgnitorState m_ignitor_state;
  PumpState m_pump_state;
  MotorState m_motor_state;
  AutomationState m_automation_state = AutomationState::STATE_IDLE;
};

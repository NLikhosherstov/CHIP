#include "system/SystemState.h"

namespace {
template <typename T>
T clampValue(T value, T min_value, T max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}
}  // namespace

SystemState::HeatExchangerState SystemState::getHeatExchangerState() const {
  noInterrupts();
  const HeatExchangerState state = m_heat_exchanger_state;
  interrupts();
  return state;
}

SystemState::RoomClimateState SystemState::getRoomClimateState() const {
  noInterrupts();
  const RoomClimateState state = m_room_climate_state;
  interrupts();
  return state;
}

SystemState::IgnitorState SystemState::getIgnitorState() const {
  noInterrupts();
  const IgnitorState state = m_ignitor_state;
  interrupts();
  return state;
}

SystemState::PumpState SystemState::getPumpState() const {
  noInterrupts();
  const PumpState state = m_pump_state;
  interrupts();
  return state;
}

SystemState::MotorState SystemState::getMotorState() const {
  noInterrupts();
  const MotorState state = m_motor_state;
  interrupts();
  return state;
}

SystemState::AutomationState SystemState::getAutomationState() const {
  noInterrupts();
  const AutomationState state = m_automation_state;
  interrupts();
  return state;
}

void SystemState::setHeatExchangerState(float temperature_c,
                                        float temp_rate_c_per_s,
                                        bool valid,
                                        uint8_t fault_flags,
                                        uint32_t updated_at_ms) {
  noInterrupts();
  m_heat_exchanger_state.temperature_c = temperature_c;
  m_heat_exchanger_state.temp_rate_c_per_s = temp_rate_c_per_s;
  m_heat_exchanger_state.valid = valid;
  m_heat_exchanger_state.fault_flags = fault_flags;
  m_heat_exchanger_state.updated_at_ms = updated_at_ms;
  interrupts();
}

void SystemState::setRoomClimateState(float temperature_c, float humidity_percent, bool valid, uint32_t updated_at_ms) {
  noInterrupts();
  m_room_climate_state.temperature_c = temperature_c;
  m_room_climate_state.humidity_percent = clampValue(humidity_percent, 0.0F, 100.0F);
  m_room_climate_state.valid = valid;
  m_room_climate_state.updated_at_ms = updated_at_ms;
  interrupts();
}

void SystemState::setIgnitorState(bool enabled, uint8_t pwm_percent, uint32_t timeout_deadline_ms) {
  noInterrupts();
  m_ignitor_state.enabled = enabled;
  m_ignitor_state.pwm_percent = clampValue<uint8_t>(pwm_percent, 0, 100);
  m_ignitor_state.timeout_deadline_ms = timeout_deadline_ms;
  interrupts();
}

void SystemState::setPumpState(bool enabled, uint8_t speed_index, uint16_t cycle_period_ms) {
  noInterrupts();
  m_pump_state.enabled = enabled;
  m_pump_state.speed_index = speed_index;
  m_pump_state.cycle_period_ms = cycle_period_ms;
  interrupts();
}

void SystemState::setMotorState(bool enabled, uint8_t speed_index, uint16_t pwm_duty_permille, uint32_t pwm_frequency_hz) {
  noInterrupts();
  m_motor_state.enabled = enabled;
  m_motor_state.speed_index = speed_index;
  m_motor_state.pwm_duty_permille = clampValue<uint16_t>(pwm_duty_permille, 0, 1000);
  m_motor_state.pwm_frequency_hz = pwm_frequency_hz;
  interrupts();
}

void SystemState::setAutomationState(AutomationState state) {
  noInterrupts();
  m_automation_state = state;
  interrupts();
}

// --- Запросы UI (выставляет DisplayManager, обрабатывает AutomationController) ---

void SystemState::postEnterAutoRequest() {
  noInterrupts();
  m_requests.enter_auto = true;
  interrupts();
}

void SystemState::postEnterManualRequest() {
  noInterrupts();
  m_requests.enter_manual = true;
  interrupts();
}

void SystemState::postEnterCalibrationRequest() {
  noInterrupts();
  m_requests.enter_calibration = true;
  interrupts();
}

void SystemState::postReloadKeyboardCalRequest() {
  noInterrupts();
  m_requests.reload_keyboard_cal = true;
  interrupts();
}

SystemState::KeyboardAdcState SystemState::getKeyboardAdcState() const {
  noInterrupts();
  const KeyboardAdcState state = m_keyboard_adc_state;
  interrupts();
  return state;
}

void SystemState::setKeyboardAdcState(uint16_t stable_adc, bool pressed) {
  noInterrupts();
  m_keyboard_adc_state.stable_adc = stable_adc;
  m_keyboard_adc_state.pressed = pressed;
  interrupts();
}

SystemState::SystemRequest SystemState::getRequests() const {
  noInterrupts();
  const SystemRequest requests = m_requests;
  interrupts();
  return requests;
}

void SystemState::clearRequestEnterAuto() {
  noInterrupts();
  m_requests.enter_auto = false;
  interrupts();
}

void SystemState::clearRequestEnterManual() {
  noInterrupts();
  m_requests.enter_manual = false;
  interrupts();
}

void SystemState::clearRequestEnterCalibration() {
  noInterrupts();
  m_requests.enter_calibration = false;
  interrupts();
}

void SystemState::clearRequestReloadKeyboardCal() {
  noInterrupts();
  m_requests.reload_keyboard_cal = false;
  interrupts();
}

void SystemState::toggleOutlineSprite() {
  noInterrupts();
  m_requests.outline_sprite = !m_requests.outline_sprite;
  interrupts();
}

bool SystemState::isOutlineSpriteEnabled() const {
  noInterrupts();
  const bool enabled = m_requests.outline_sprite;
  interrupts();
  return enabled;
}

void SystemState::clearAllRequests() {
  noInterrupts();
  m_requests = SystemRequest{};
  interrupts();
}

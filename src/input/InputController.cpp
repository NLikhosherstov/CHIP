#include "input/InputController.h"

#include "automation/AutomationController.h"
#include "display/DisplayManager.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

InputController::InputController(DisplayManager& display,
                                 AutomationController& automation,
                                 SystemState& state,
                                 ConfigManager& config)
    : m_display(display),
      m_automation(automation),
      m_state(state),
      m_config(config) {}

void InputController::begin() {
  m_hal.begin(m_config.getConfig());
}

void InputController::tick() {
  const bool cal = m_display.isCalibrationActive();
  m_hal.setCalibrationMode(cal);

  m_hal.tick();

  if (cal) {
    m_state.setKeyboardAdcState(m_hal.getStableAdc(), m_hal.isKeyPressed());
  }

  const auto req = m_state.getRequests();
  if (req.reload_keyboard_cal) {
    m_state.clearRequestReloadKeyboardCal();
    m_hal.reloadCalibration(m_config.getConfig());
    m_hal.setCalibrationMode(m_display.isCalibrationActive());
  }

  RawInputEvent ev{};
  while (m_hal.pop(ev)) {
    dispatch(ev);
  }
}

bool InputController::mapRawToEvent(const RawInputEvent& ev, EventType& out, int16_t& payload) const {
  if (ev.kind == RawInputEvent::Kind::Button) {
    const bool long_press = ev.button.gesture == ButtonGesture::LongPress;
    switch (ev.button.btn) {
      case PhysicalButton::Power:
        out = long_press ? EventType::PowerLong : EventType::Power;
        payload = 0;
        return true;
      case PhysicalButton::Up:
        out = long_press ? EventType::UpLong : EventType::Up;
        payload = 0;
        return true;
      case PhysicalButton::Down:
        out = long_press ? EventType::DownLong : EventType::Down;
        payload = 0;
        return true;
      case PhysicalButton::Pump:
        out = long_press ? EventType::LeftLong : EventType::Left;
        payload = 0;
        return true;
      case PhysicalButton::Ignitor:
        out = long_press ? EventType::RightLong : EventType::Right;
        payload = 0;
        return true;
      default:
        return false;
    }
  }

  if (ev.kind != RawInputEvent::Kind::Encoder) {
    return false;
  }

  switch (ev.encoder.motion) {
    case EncoderMotion::StepCw:
    case EncoderMotion::StepCcw:
      out = EventType::Delta;
      payload = ev.encoder.steps;
      return true;
    case EncoderMotion::KeyClick:
      out = EventType::Click;
      payload = 0;
      return true;
    case EncoderMotion::KeyLongPress:
      out = EventType::LongClick;
      payload = 0;
      return true;
    default:
      return false;
  }
}

void InputController::dispatch(const RawInputEvent& ev) {
  EventType type{};
  int16_t payload = 0;
  if (!mapRawToEvent(ev, type, payload)) {
    return;
  }

  Event e{type, payload, false, m_state, m_config};
  m_display.event(e);
  if (!e.accept) {
    m_automation.event(e);
  }
}

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

bool InputController::mapButtonToUi(const RawInputEvent& ev, UiInput& out) const {
  if (ev.kind != RawInputEvent::Kind::Button ||
      ev.button.gesture != ButtonGesture::Click) {
    return false;
  }

  switch (ev.button.btn) {
    case PhysicalButton::Up:
      out = UiInput::Up;
      return true;
    case PhysicalButton::Down:
      out = UiInput::Down;
      return true;
    case PhysicalButton::Pump:
      out = UiInput::Left;
      return true;
    case PhysicalButton::Ignitor:
      out = UiInput::Right;
      return true;
    default:
      return false;
  }
}

bool InputController::mapEncoderToUi(const RawInputEvent& ev, UiInput& out, int16_t& payload) const {
  if (ev.kind != RawInputEvent::Kind::Encoder) {
    return false;
  }

  switch (ev.encoder.motion) {
    case EncoderMotion::StepCw:
      out = UiInput::Delta;
      payload = ev.encoder.steps;
      return true;
    case EncoderMotion::StepCcw:
      out = UiInput::Delta;
      payload = ev.encoder.steps;
      return true;
    case EncoderMotion::KeyClick:
      out = UiInput::Click;
      payload = 0;
      return true;
    case EncoderMotion::KeyLongPress:
      out = UiInput::LongClick;
      payload = 0;
      return true;
    default:
      return false;
  }
}

void InputController::routeUi(UiInput action, int16_t payload) {
  m_display.handleUiInput(action, payload, m_state, m_config);
}

void InputController::routeAutomation(AutomationCommand cmd, int16_t payload) {
  switch (cmd) {
    case AutomationCommand::StartAlgorithm:
      m_automation.requestStart();
      break;
    case AutomationCommand::StopAlgorithm:
      m_automation.requestStop();
      break;
    case AutomationCommand::EmergencyStop:
      m_automation.emergencyStop();
      break;
    case AutomationCommand::EnterAuto:
      m_automation.enterAuto();
      break;
    case AutomationCommand::EnterManual:
      m_automation.enterManual();
      break;
    case AutomationCommand::SetMotorStep:
      m_automation.setMotorStep(static_cast<int8_t>(payload));
      break;
    case AutomationCommand::SetPumpEnabled: {
      const auto pump = m_state.getPumpState();
      m_automation.setPumpEnabled(!pump.enabled);
      break;
    }
    case AutomationCommand::SetIgnitorEnabled: {
      const auto ign = m_state.getIgnitorState();
      m_automation.setIgnitorEnabled(!ign.enabled);
      break;
    }
  }
}

void InputController::dispatch(const RawInputEvent& ev) {
  // Проверка калибровочного режима
  if (m_display.isCalibrationActive()) {
    if (ev.kind == RawInputEvent::Kind::Encoder) {
      UiInput ui_action{};
      int16_t payload = 0;
      if (mapEncoderToUi(ev, ui_action, payload)) {
        routeUi(ui_action, payload);
      }
    }
    return;
  }

  // Power: в главном меню — выход без сохранения; иначе — автоматика.
  if (ev.kind == RawInputEvent::Kind::Button) {
    if (ev.button.btn == PhysicalButton::Power) {
      if (m_display.currentScreen() == DisplayManager::ScreenId::MAIN_MENU &&
          ev.button.gesture == ButtonGesture::Click) {
        routeUi(UiInput::Power, 0);
        return;
      }
      if (ev.button.gesture == ButtonGesture::LongPress) {
        routeAutomation(AutomationCommand::EmergencyStop);
      } else if (ev.button.gesture == ButtonGesture::Click) {
        const auto st = m_state.getAutomationState();
        if (st == SystemState::AutomationState::STATE_IDLE) {
          routeAutomation(AutomationCommand::StartAlgorithm);
        } else {
          routeAutomation(AutomationCommand::StopAlgorithm);
        }
      }
      return;
    }
  }

  if (ev.kind == RawInputEvent::Kind::Encoder && ev.encoder.motion == EncoderMotion::KeyClick) {
    routeUi(UiInput::OpenMainMenu, 0);
    return;
  }

  UiInput ui_action{};
  int16_t payload = 0;

  if (ev.kind == RawInputEvent::Kind::Button) {
    if (!mapButtonToUi(ev, ui_action)) {
      return;
    }
  } else if (!mapEncoderToUi(ev, ui_action, payload)) {
    return;
  }

  // В меню весь семантический ввод — в DisplayManager.
  if (m_display.isMenuActive()) {
    routeUi(ui_action, payload);
    return;
  }

  // HOME без меню: Up/Down открывают быстрое меню.
  if (ui_action == UiInput::Up || ui_action == UiInput::Down) {
    routeUi(ui_action, payload);
    return;
  }

  // HOME: Насос/Свеча — прямое управление исполнителями.
  if (ui_action == UiInput::Left) {
    routeAutomation(AutomationCommand::SetPumpEnabled, 1);
    return;
  }
  if (ui_action == UiInput::Right) {
    routeAutomation(AutomationCommand::SetIgnitorEnabled, 1);
    return;
  }

  if (ui_action == UiInput::LongClick) {
    if (m_state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL) {
      routeAutomation(AutomationCommand::EnterAuto);
    }
    return;
  }

  if (ui_action == UiInput::Delta) {
    const auto motor = m_state.getMotorState();
    int16_t step = static_cast<int16_t>(motor.speed_index) + payload;
    routeAutomation(AutomationCommand::SetMotorStep, step);
    return;
  }

  if (ui_action == UiInput::OpenMainMenu) {
    routeUi(UiInput::OpenMainMenu, 0);
  }
}

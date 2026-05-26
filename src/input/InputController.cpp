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
  m_hal.tick();

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
      out = UiInput::OpenMainMenu;
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
      m_automation.setMotorStep(static_cast<uint8_t>(payload));
      break;
    case AutomationCommand::SetPumpEnabled: {
      m_automation.enterManual();
      const auto pump = m_state.getPumpState();
      m_automation.setPumpEnabled(!pump.enabled);
      break;
    }
    case AutomationCommand::SetIgnitorEnabled: {
      m_automation.enterManual();
      const auto ign = m_state.getIgnitorState();
      m_automation.setIgnitorEnabled(!ign.enabled);
      break;
    }
  }
}

void InputController::dispatch(const RawInputEvent& ev) {
  // Power обрабатывается только автоматикой.
  if (ev.kind == RawInputEvent::Kind::Button) {
    if (ev.button.btn == PhysicalButton::Power) {
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

  if (ev.kind == RawInputEvent::Kind::Encoder && ev.encoder.motion == EncoderMotion::KeyLongPress) {
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

  if (ui_action == UiInput::Click) {
    if (m_state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL) {
      routeAutomation(AutomationCommand::EnterAuto);
    }
    return;
  }

  if (ui_action == UiInput::Delta) {
    if (m_state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL) {
      const auto motor = m_state.getMotorState();
      int16_t step = static_cast<int16_t>(motor.speed_index + 1) + payload;
      if (step < 1) {
        step = 1;
      }
      if (step > 4) {
        step = 4;
      }
      routeAutomation(AutomationCommand::SetMotorStep, step);
    }
    return;
  }

  if (ui_action == UiInput::OpenMainMenu) {
    routeUi(UiInput::OpenMainMenu, 0);
  }
}

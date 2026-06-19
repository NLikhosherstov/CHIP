#include "automation/AutomationController.h"

#include "system/ConfigManager.h"

AutomationController::AutomationController(SystemState& state, ConfigManager& config)
    : m_state(state),
      m_config(config),
      m_pump(state, config),
      m_motor(state, config),
      m_ignitor(state, config) {}

void AutomationController::begin() {
  m_pump.begin();
  m_motor.begin();
  m_ignitor.begin();
  m_state.setAutomationState(SystemState::AutomationState::STATE_IDLE);
}

void AutomationController::tick(uint32_t /*now_ms*/) {
  processRequests();
}

void AutomationController::event(Event& e) {
  switch (e.type) {
    case EventType::PowerLong:
      emergencyStop();
      break;
    case EventType::Power: {
      const auto st = e.state.getAutomationState();
      if (st == SystemState::AutomationState::STATE_IDLE) {
        requestStart();
      } else {
        requestStop();
      }
      break;
    }
    case EventType::Left: {
      const auto pump = e.state.getPumpState();
      if (pump.speed_index > 0) {
        setPumpStep(0);
      } else {
        const auto motor = e.state.getMotorState();
        if (motor.speed_index > 0)
          setPumpStep(motor.speed_index);
        else
          setPumpStep(4);
      }
      break;
    }
    case EventType::Right: {
      const auto ign = e.state.getIgnitorState();
      setIgnitorEnabled(!ign.enabled);
      break;
    }
    case EventType::LongClick:
      if (e.state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL) {
        enterAuto();
      }
      break;
    case EventType::Delta: {
      const auto motor = e.state.getMotorState();
      const int16_t step = static_cast<int16_t>(motor.speed_index) + e.payload;
      setMotorStep(static_cast<int8_t>(step));
      break;
    }
    default:
      break;
  }
}

// Запросы от UI (быстрое меню «Режим» и т.п.) — без прямой связи DisplayManager ↔ AutomationController.
void AutomationController::processRequests() {
  const SystemState::SystemRequest req = m_state.getRequests();

  if (req.enter_auto) {
    enterAuto();
    m_state.clearRequestEnterAuto();
  }
  if (req.enter_manual) {
    enterManual();
    m_state.clearRequestEnterManual();
  }
}

void AutomationController::requestStart() {
  const auto st = m_state.getAutomationState();
  if (st == SystemState::AutomationState::STATE_IDLE) {
    m_state.setAutomationState(SystemState::AutomationState::STATE_AUTO_START);
  }
}

void AutomationController::requestStop() {
  const auto st = m_state.getAutomationState();
  if (st != SystemState::AutomationState::STATE_IDLE &&
      st != SystemState::AutomationState::STATE_STOP) {
    m_state.setAutomationState(SystemState::AutomationState::STATE_STOP);
  }
}

void AutomationController::emergencyStop() {
  m_pump.setStep(0);
  m_motor.setStep(0);
  m_ignitor.setEnabled(false);
  m_state.setAutomationState(SystemState::AutomationState::STATE_STOP);
}

void AutomationController::enterAuto() {
  const auto st = m_state.getAutomationState();
  if (st == SystemState::AutomationState::STATE_MANUAL) {
    m_state.setAutomationState(SystemState::AutomationState::STATE_AUTO_TSTAT);
  }
}

void AutomationController::enterManual() {
  m_state.setAutomationState(SystemState::AutomationState::STATE_MANUAL);
}

void AutomationController::setMotorStep(int8_t step_0_to_4) {
  const auto pump = m_state.getPumpState();
  const auto motor = m_state.getMotorState();
  bool isSync = pump.speed_index == motor.speed_index;
  bool isMotorStart = ( motor.speed_index == 0 && step_0_to_4 );
  
  m_motor.setStep(step_0_to_4);
  
  // Синхронно меняем скорость насоса если он включен,
  // и находится в режиме синхронной работы с мотором
  // или производится старт двигателя с включенным насосом
  if(pump.enabled && (isSync || isMotorStart))
    m_pump.setStep(step_0_to_4);
}

void AutomationController::setPumpStep(int8_t step_0_to_4) {
  m_pump.setStep(step_0_to_4);
}

void AutomationController::setIgnitorEnabled(bool on) {
  m_ignitor.setEnabled(on);
}

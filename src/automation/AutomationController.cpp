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
  m_pump.setEnabled(false);
  m_motor.setEnabled(false);
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

void AutomationController::setMotorStep(uint8_t step_1_to_4) {
  m_motor.setStep(step_1_to_4);
}

void AutomationController::setPumpEnabled(bool on) {
  m_pump.setEnabled(on);
}

void AutomationController::setIgnitorEnabled(bool on) {
  m_ignitor.setEnabled(on);
}

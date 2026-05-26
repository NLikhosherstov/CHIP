#pragma once

#include "display/UiInput.h"
#include "input/InputHal.h"
#include "input/InputTypes.h"

class AutomationController;
class ConfigManager;
class DisplayManager;
class SystemState;

// Шлюз ввода: физика → UiInput / AutomationCommand.
// Не знает деталей меню; контекст запрашивает у DisplayManager::isMenuActive().
class InputController {
public:
  InputController(DisplayManager& display,
                  AutomationController& automation,
                  SystemState& state,
                  ConfigManager& config);

  void begin();
  void tick();

private:
  void dispatch(const RawInputEvent& ev);
  void routeUi(UiInput action, int16_t payload);
  void routeAutomation(AutomationCommand cmd, int16_t payload = 0);

  bool mapButtonToUi(const RawInputEvent& ev, UiInput& out) const;
  bool mapEncoderToUi(const RawInputEvent& ev, UiInput& out, int16_t& payload) const;

  DisplayManager& m_display;
  AutomationController& m_automation;
  SystemState& m_state;
  ConfigManager& m_config;
  InputHal m_hal;
};

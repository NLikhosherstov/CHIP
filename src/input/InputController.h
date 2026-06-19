#pragma once

#include "input/Event.h"
#include "input/InputHal.h"
#include "input/InputTypes.h"

class AutomationController;
class ConfigManager;
class DisplayManager;
class SystemState;

// Шлюз ввода: физика → Event, цепочка потребителей.
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

  bool mapRawToEvent(const RawInputEvent& ev, EventType& out, int16_t& payload) const;

  DisplayManager& m_display;
  AutomationController& m_automation;
  SystemState& m_state;
  ConfigManager& m_config;
  InputHal m_hal;
};

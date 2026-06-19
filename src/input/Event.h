#pragma once

#include <stdint.h>

class ConfigManager;
class SystemState;

// Семантические жесты ввода после маппинга из RawInputEvent.
enum class EventType : uint8_t {
  Power,
  PowerLong,
  Up,
  UpLong,
  Down,
  DownLong,
  Left,
  LeftLong,
  Right,
  RightLong,
  Click,
  LongClick,
  Delta,

  Pump     = Left,
  Ignition = Right,
};

struct Event {
  EventType type;
  int16_t payload;
  bool accept;
  SystemState& state;
  ConfigManager& config;
};

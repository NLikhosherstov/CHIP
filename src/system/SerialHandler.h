#pragma once

#include <stdint.h>

class ConfigManager;
class SystemState;

class SerialHandler {
public:
  SerialHandler(SystemState& state, ConfigManager& config);

  void tick();

private:
  void processLine(char* line);
  void handleConnectionChange(bool connected);
  void printWelcomeMessage() const;

  void printState() const;
  void printHeatExchanger() const;
  void printRoomClimate() const;
  void printIgnitor() const;
  void printPump() const;
  void printMotor() const;
  void printAutomationMode() const;
  void printConfig() const;
  void printCommandHelp() const;
  void toggleOutlineSprite();

  static void rebootToTinyUF2();
  static void restartFirmware();

  SystemState& m_state;
  ConfigManager& m_config;

  char m_line_buffer[64];
  uint8_t m_line_len = 0;
  bool m_was_connected = false;
  bool m_has_logged_welcome = false;
};

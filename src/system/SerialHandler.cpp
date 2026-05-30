#include "system/SerialHandler.h"

#include <Arduino.h>
#include <string.h>

#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

void trimLine(char* line) {
  if (line == nullptr) {
    return;
  }

  char* start = line;
  while (*start == ' ' || *start == '\t') {
    ++start;
  }

  if (start != line) {
    memmove(line, start, strlen(start) + 1);
  }

  const size_t len = strlen(line);
  size_t end = len;
  while (end > 0 && (line[end - 1] == ' ' || line[end - 1] == '\t')) {
    --end;
  }
  line[end] = '\0';
}

const char* automationModeName(SystemState::AutomationState state) {
  switch (state) {
    case SystemState::AutomationState::STATE_IDLE:
      return "IDLE";
    case SystemState::AutomationState::STATE_AUTO_START:
      return "AUTO";
    case SystemState::AutomationState::STATE_AUTO_TSTAT:
      return "AUTO";
    case SystemState::AutomationState::STATE_MANUAL:
      return "MANUAL";
    case SystemState::AutomationState::STATE_STOP:
      return "STOP";
    default:
      return "----";
  }
}

void printLabelBool(const __FlashStringHelper* label, bool value) {
  Serial.print(label);
  Serial.println(value ? 1 : 0);
}

void printLabelInt(const __FlashStringHelper* label, int32_t value) {
  Serial.print(label);
  Serial.println(value);
}

void printLabelUInt(const __FlashStringHelper* label, uint32_t value) {
  Serial.print(label);
  Serial.println(value);
}

void printLabelFloat(const __FlashStringHelper* label, float value, uint8_t decimals) {
  Serial.print(label);
  Serial.println(value, decimals);
}

void printLabelHex8(const __FlashStringHelper* label, uint8_t value) {
  Serial.print(label);
  Serial.print(F("0x"));
  if (value < 0x10) {
    Serial.print('0');
  }
  Serial.println(value, HEX);
}

void printCommandSeparator() {
  Serial.println(F("=========================="));
  Serial.println();
}

}  // namespace

SerialHandler::SerialHandler(SystemState& state, ConfigManager& config)
    : m_state(state), m_config(config) {}

void SerialHandler::tick() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\n') {
      m_line_buffer[m_line_len] = '\0';
      processLine(m_line_buffer);
      m_line_len = 0;
    } else if (c != '\r') {
      if (m_line_len < sizeof(m_line_buffer) - 1) {
        m_line_buffer[m_line_len++] = c;
      } else {
        m_line_len = 0;
      }
    }
  }

  const bool is_connected = static_cast<bool>(Serial);
  if (is_connected != m_was_connected) {
    handleConnectionChange(is_connected);
    m_was_connected = is_connected;
  }
}

void SerialHandler::processLine(char* line) {
  trimLine(line);
  if (line[0] == '\0') {
    return;
  }

  if (strstr(line, "BOOT") != nullptr) {
    digitalWrite(PC13, HIGH);
    rebootToTinyUF2();
    return;
  }

  if (strcmp(line, "restart") == 0) {
    restartFirmware();
    return;
  }

  if (strcmp(line, "state") == 0) {
    printState();
  } else if (strcmp(line, "exc") == 0) {
    printHeatExchanger();
  } else if (strcmp(line, "room") == 0) {
    printRoomClimate();
  } else if (strcmp(line, "ign") == 0) {
    printIgnitor();
  } else if (strcmp(line, "pump") == 0) {
    printPump();
  } else if (strcmp(line, "motor") == 0) {
    printMotor();
  } else if (strcmp(line, "mode") == 0) {
    printAutomationMode();
  } else if (strcmp(line, "cfg") == 0) {
    printConfig();
  } else if (strcmp(line, "outline") == 0) {
    toggleOutlineSprite();
  } else if (strcmp(line, "help") == 0) {
    printCommandHelp();
  } else {
    printCommandHelp();
  }

  printCommandSeparator();
}

void SerialHandler::handleConnectionChange(bool connected) {
  if (connected) {
    if (!m_has_logged_welcome) {
      printWelcomeMessage();
      m_has_logged_welcome = true;
    }
  } else {
    m_has_logged_welcome = false;
  }
}

void SerialHandler::printWelcomeMessage() const {
  Serial.println(F("\n===================================="));
  Serial.println(F("Config loading status:"));

  if (m_config.isLoaded()) {
    Serial.println(F("  - SUCCESS: User config loaded from Flash/EEPROM."));
    if (m_config.getConfig().btnPower == 0) {
      Serial.println(F("  - KBD: Calibration required!"));
    }
  } else {
    Serial.println(F("  - WARNING: Flash was empty/corrupted. Default settings applied."));
    Serial.println(F("  - KBD: Calibration required!"));
  }
  Serial.println(F("===================================="));
}

void SerialHandler::printHeatExchanger() const {
  const auto s = m_state.getHeatExchangerState();
  printLabelFloat(F("temperature_c       - "), s.temperature_c, 2);
  printLabelFloat(F("temp_rate_c_per_s   - "), s.temp_rate_c_per_s, 2);
  printLabelBool( F("valid               - "), s.valid);
  printLabelHex8( F("fault_flags         - "), s.fault_flags);
  printLabelUInt( F("updated_at_ms       - "), s.updated_at_ms);
}

void SerialHandler::printRoomClimate() const {
  const auto s = m_state.getRoomClimateState();
  printLabelFloat(F("temperature_c       - "), s.temperature_c, 2);
  printLabelFloat(F("humidity_percent    - "), s.humidity_percent, 2);
  printLabelBool( F("valid               - "), s.valid);
  printLabelUInt( F("updated_at_ms       - "), s.updated_at_ms);
}

void SerialHandler::printIgnitor() const {
  const auto s = m_state.getIgnitorState();
  printLabelBool(F("enabled             - "), s.enabled);
  printLabelInt( F("pwm_percent         - "), s.pwm_percent);
  printLabelUInt(F("timeout_deadline_ms - "), s.timeout_deadline_ms);
}

void SerialHandler::printPump() const {
  const auto s = m_state.getPumpState();
  printLabelBool(F("enabled             - "), s.enabled);
  printLabelInt( F("speed_index         - "), s.speed_index);
  printLabelUInt(F("pulse_hz            - "), s.pulse_hz);
}

void SerialHandler::printMotor() const {
  const auto s = m_state.getMotorState();
  printLabelBool(F("enabled             - "), s.enabled);
  printLabelInt( F("speed_index         - "), s.speed_index);
  printLabelUInt(F("pwm_duty_permille   - "), s.pwm_duty_permille);
  printLabelUInt(F("pwm_frequency_hz    - "), s.pwm_frequency_hz);
}

void SerialHandler::printAutomationMode() const {
  Serial.print(F("automation - "));
  Serial.println(automationModeName(m_state.getAutomationState()));
}

void SerialHandler::printState() const {
  Serial.println(F("--- MAX31855 ---"));
  printHeatExchanger();
  Serial.println();
  Serial.println(F("--- DHT22 ---"));
  printRoomClimate();
  Serial.println();
  Serial.println(F("--- Ignitor state ---"));
  printIgnitor();
  Serial.println();
  Serial.println(F("--- Pump state ---"));
  printPump();
  Serial.println();
  Serial.println(F("--- Motor state ---"));
  printMotor();
}

void SerialHandler::printConfig() const {
  const ConfigManager::Config& cfg = m_config.getConfig();

  Serial.println(F("--- Keyboard Calibration ---"));
  printLabelUInt(F("btnPower        - "), cfg.btnPower);
  printLabelUInt(F("btnUp           - "), cfg.btnUp);
  printLabelUInt(F("btnIgnition     - "), cfg.btnIgnition);
  printLabelUInt(F("btnPump         - "), cfg.btnPump);
  printLabelUInt(F("btnDown         - "), cfg.btnDown);

  Serial.println();
  Serial.println(F("--- Pump ---"));
  printLabelUInt(F("pump_pulse_1    - "), cfg.pump_pulse_1);
  printLabelUInt(F("pump_pulse_2    - "), cfg.pump_pulse_2);
  printLabelUInt(F("pump_pulse_3    - "), cfg.pump_pulse_3);
  printLabelUInt(F("pump_pulse_4    - "), cfg.pump_pulse_4);
  printLabelUInt(F("pumpPerfomance  - "), cfg.pumpPerfomance);
  printLabelInt( F("fuel_correction - "), cfg.fuel_correction);
  printLabelBool(F("embededPump     - "), cfg.embededPump);

  Serial.println();
  Serial.println(F("--- Motor ---"));
  printLabelInt( F("motor_PWM_1    - "), cfg.motor_PWM_1);
  printLabelInt( F("motor_PWM_2    - "), cfg.motor_PWM_2);
  printLabelInt( F("motor_PWM_3    - "), cfg.motor_PWM_3);
  printLabelInt( F("motor_PWM_4    - "), cfg.motor_PWM_4);

  Serial.println();
  Serial.println(F("--- Other ---"));
  printLabelUInt(F("ignitor_timeout_s    - "), cfg.ignitor_timeout_s);
  printLabelInt( F("target_temperature_c - "), cfg.target_temperature_c);
}

void SerialHandler::printCommandHelp() const {
  Serial.println(F("Commands:"));
  Serial.println(F("  BOOT    - reboot into TinyUF2 bootloader (used by uf2conv upload script)"));
  Serial.println(F("  state   - all sensor/actuator states"));
  Serial.println(F("  exc     - heat exchanger (MAX31855)"));
  Serial.println(F("  room    - room climate (DHT22)"));
  Serial.println(F("  ign     - ignitor state"));
  Serial.println(F("  pump    - pump state"));
  Serial.println(F("  motor   - motor state"));
  Serial.println(F("  mode    - automation mode (IDLE/AUTO/MANUAL/STOP)"));
  Serial.println(F("  cfg     - configuration by category"));
  Serial.println(F("  outline - toggle sprite perimeter outline when rendering"));
  Serial.println(F("  restart - firmware restart"));
  Serial.println(F("  help    - this list"));
}

void SerialHandler::toggleOutlineSprite() {
  m_state.toggleOutlineSprite();
  printLabelBool(F("outline_sprite - "), m_state.isOutlineSpriteEnabled());
}

void SerialHandler::restartFirmware() {
  NVIC_SystemReset();
}

void SerialHandler::rebootToTinyUF2() {
  constexpr uint32_t DBL_TAP_MAGIC = 0xf01669efUL;
  constexpr uint32_t DBL_TAP_ADDR = 0x20000000 + 64 * 1024 - 4;

  __disable_irq();
  volatile uint32_t* boot_mode = reinterpret_cast<volatile uint32_t*>(DBL_TAP_ADDR);
  *boot_mode = DBL_TAP_MAGIC;

  asm volatile("dsb");
  NVIC_SystemReset();
}

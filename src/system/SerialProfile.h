#pragma once

#include <Arduino.h>

// Host-side wall-clock profiling via Serial markers:
//   @PROFILE <traceName> BEGIN
//   @PROFILE <traceName> END
// traceName must be a single token (no spaces). Pair with tools/serial_profile.py.

class SerialProfileScope {
public:
  explicit SerialProfileScope(const __FlashStringHelper* name)
      : m_name(name), m_active(true) {
    emit(m_name, F("BEGIN"));
  }

  ~SerialProfileScope() {
    if (m_active) {
      emit(m_name, F("END"));
    }
  }

  SerialProfileScope(const SerialProfileScope&) = delete;
  SerialProfileScope& operator=(const SerialProfileScope&) = delete;

  void dismiss() { m_active = false; }

  static void begin(const __FlashStringHelper* name) { emit(name, F("BEGIN")); }
  static void end(const __FlashStringHelper* name) { emit(name, F("END")); }

private:
  static void emit(const __FlashStringHelper* name, const __FlashStringHelper* phase) {
    Serial.print(F("@PROFILE "));
    Serial.print(name);
    Serial.print(F(" "));
    Serial.println(phase);
    Serial.flush();
  }

  const __FlashStringHelper* m_name;
  bool m_active;
};

#define PROFILE_SCOPE(name) SerialProfileScope _profile_##__LINE__(F(name))

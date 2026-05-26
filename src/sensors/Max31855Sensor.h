#pragma once

#include <Adafruit_SPIDevice.h>
#include <Arduino.h>

#include "system/SystemState.h"

class Max31855Sensor {
public:
  explicit Max31855Sensor(SystemState& state);

  void begin();
  void tick();
  void setTracingEnabled(bool enabled);

private:
  struct Sample {
    uint32_t word = 0;
    uint8_t fault_flags = 0;
    float raw_c = 0.0F;
  };

  // Период опроса датчика (мс). 250 мс => 4 Гц.
  static constexpr uint32_t kPollIntervalMs = 250;

  // Коэффициент EMA фильтра температуры.
  static constexpr float kFilterAlpha = 0.45F;

  // Коэффициент сглаживания производной dT/dt.
  static constexpr float kRateBeta = 0.35F;
  
  // Макс. шаг raw за один отсчет (~16 °C/s при 4 Гц).
  static constexpr float kMaxRawStepC = 4.0F;
  
  // Подряд отклоненных выбросов перед принятием скачка.
  static constexpr uint8_t kOutlierAcceptStrikes = 2;
  
  // Ограничение производной (физически правдоподобный диапазон).
  static constexpr float kRateLimitCPerS = 30.0F;
  
  // Программно допустимый диапазон температуры для выхлопа.
  static constexpr float kValidMinTempC = -50.0F;
  static constexpr float kValidMaxTempC = 900.0F;
  
  // Кол-во подряд ошибочных отсчетов для перехода в invalid.
  static constexpr uint8_t kFaultStrikes = 3;
  
  // Кол-во подряд хороших отсчетов для восстановления valid.
  static constexpr uint8_t kRecoverStrikes = 2;
  
  // Внутренний флаг: температура вне допустимого программного диапазона.
  static constexpr uint8_t kFaultRange = 0x80;
  
  // Plateau snap: |raw - filt| ниже порога.
  static constexpr float kPlateauSnapDeltaC = 0.5F;
  
  // Plateau snap: стабильность raw между отсчетами.
  static constexpr float kPlateauRawStableC = 0.25F;
  
  // Plateau snap: число стабильных отсчетов подряд.
  static constexpr uint8_t kPlateauStableTicks = 4;

  bool readSample(Sample& sample);
  void publishState(uint32_t now_ms) const;
  bool updateFilteredTemperature(float raw_c);
  void tryPlateauSnap(float raw_c);
  void updateTemperatureRate(uint32_t now_ms);
  void traceSample(float raw_c, float rate_for_trace) const;
  static float clampFloat(float value, float min_value, float max_value);
  static float decodeCelsiusFromWord(uint32_t word);

private:
  SystemState& m_state;
  Adafruit_SPIDevice m_spi;

private:
  uint32_t m_last_poll_ms = 0;
  uint32_t m_prev_rate_ts_ms = 0;
  
  float m_filtered_c = 0.0F;
  float m_prev_rate_temp_c = 0.0F;
  float m_rate_c_per_s = 0.0F;
  float m_prev_accepted_raw_c = 0.0F;
  float m_prev_raw_c = 0.0F;
  
  uint8_t m_fault_flags = 0;
  uint8_t m_fault_streak = 0;
  uint8_t m_recover_streak = 0;
  uint8_t m_outlier_streak = 0;
  uint8_t m_plateau_ticks = 0;
  
  bool m_is_started = false;
  bool m_is_valid = false;
  bool m_has_filtered = false;
  bool m_has_rate_reference = false;
  bool m_tracing_enabled = false;
};

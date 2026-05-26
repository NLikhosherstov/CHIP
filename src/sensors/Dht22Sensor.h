#pragma once

#include <Arduino.h>
#include <DHT.h>

#include "system/SystemState.h"

class Dht22Sensor {
public:
  explicit Dht22Sensor(SystemState& state);

  void begin();
  void tick();

private:
  // Минимальный интервал опроса DHT22/AM2302 (мс). Датчик не чаще ~0.5 Гц.
  static constexpr uint32_t kPollIntervalMs = 2000;

  // Размер окна скользящего среднего (число успешных отсчётов).
  static constexpr uint8_t kMovingAverageWindow = 2;

  // Шаг квантования температуры (°C) — целевая точность отображения/состояния.
  static constexpr float kTemperatureQuantizeC = 0.5F;

  // Шаг квантования влажности (%).
  static constexpr float kHumidityQuantizePercent = 0.5F;

  // Допустимый диапазон температуры DHT22 (°C).
  static constexpr float kValidMinTempC = -40.0F;
  static constexpr float kValidMaxTempC = 80.0F;

  // Подряд ошибочных отсчётов до перехода в invalid.
  static constexpr uint8_t kFaultStrikes = 3;

  // Подряд успешных отсчётов для восстановления valid.
  static constexpr uint8_t kRecoverStrikes = 2;

  bool readSample(float& temperature_c, float& humidity_percent);
  void pushMovingAverage(float temperature_c, float humidity_percent);
  void computeMovingAverage(float& temperature_c, float& humidity_percent) const;
  void publishState(uint32_t now_ms) const;
  static float quantizeStep(float value, float step);
  static bool isSampleValid(float temperature_c, float humidity_percent);

private:
  SystemState& m_state;
  DHT m_dht;

  float m_temp_samples[kMovingAverageWindow] = {};
  float m_humidity_samples[kMovingAverageWindow] = {};
  uint8_t m_sample_count = 0;
  uint8_t m_sample_index = 0;

  float m_filtered_temp_c = 0.0F;
  float m_filtered_humidity_percent = 0.0F;

  uint32_t m_last_poll_ms = 0;
  uint8_t m_fault_streak = 0;
  uint8_t m_recover_streak = 0;

  bool m_is_started = false;
  bool m_is_valid = false;
  bool m_has_filtered = false;
};

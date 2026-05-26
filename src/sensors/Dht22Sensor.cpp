#include "sensors/Dht22Sensor.h"

#include <math.h>

#include "pins.h"

Dht22Sensor::Dht22Sensor(SystemState& state)
    : m_state(state),
      m_dht(pin::DHT_DATA, DHT22) {
}

void Dht22Sensor::begin() {
  m_is_started = true;
  m_dht.begin();
  m_last_poll_ms = millis();
  m_is_valid = false;
  m_has_filtered = false;
  m_sample_count = 0;
  m_sample_index = 0;
  m_fault_streak = 0;
  m_recover_streak = 0;

  publishState(m_last_poll_ms);
}

void Dht22Sensor::tick() {
  if(!m_is_started){
    return;
  }

  // Периодический опрос с учётом минимального интервала DHT22.
  const uint32_t now_ms = millis();
  if((now_ms - m_last_poll_ms) < kPollIntervalMs){
    return;
  }
  m_last_poll_ms = now_ms;

  float raw_temp_c = 0.0F;
  float raw_humidity_percent = 0.0F;
  if(!readSample(raw_temp_c, raw_humidity_percent)){
    // Ошибка чтения: накапливаем счётчик и при необходимости сбрасываем valid.
    if(m_fault_streak < 255){
      ++m_fault_streak;
    }
    m_recover_streak = 0;

    if(m_fault_streak >= kFaultStrikes){
      m_is_valid = false;
      m_has_filtered = false;
      m_sample_count = 0;
      m_sample_index = 0;
    }

    publishState(now_ms);
    return;
  }

  // Успешный отсчёт: сбрасываем ошибки, восстанавливаем valid после серии удачных чтений.
  m_fault_streak = 0;
  if(m_recover_streak < 255){
    ++m_recover_streak;
  }

  if(!m_is_valid){
    if(m_recover_streak < kRecoverStrikes){
      publishState(now_ms);
      return;
    }

    m_is_valid = true;
    m_has_filtered = false;
    m_sample_count = 0;
    m_sample_index = 0;
  }

  // Скользящее среднее по окну последних успешных измерений.
  pushMovingAverage(raw_temp_c, raw_humidity_percent);
  computeMovingAverage(m_filtered_temp_c, m_filtered_humidity_percent);
  m_has_filtered = (m_sample_count > 0);

  publishState(now_ms);
}

bool Dht22Sensor::readSample(float& temperature_c, float& humidity_percent) {
  // readHumidity() выполняет единственный цикл чтения; температура из того же отсчёта.
  humidity_percent = m_dht.readHumidity();
  temperature_c = m_dht.readTemperature();

  return isSampleValid(temperature_c, humidity_percent);
}

bool Dht22Sensor::isSampleValid(float temperature_c, float humidity_percent) {
  if(isnan(temperature_c) || isnan(humidity_percent)){
    return false;
  }
  if(temperature_c < kValidMinTempC || temperature_c > kValidMaxTempC){
    return false;
  }
  if(humidity_percent < 0.0F || humidity_percent > 100.0F){
    return false;
  }
  return true;
}

void Dht22Sensor::pushMovingAverage(float temperature_c, float humidity_percent) {
  m_temp_samples[m_sample_index] = temperature_c;
  m_humidity_samples[m_sample_index] = humidity_percent;

  m_sample_index = static_cast<uint8_t>((m_sample_index + 1U) % kMovingAverageWindow);
  if(m_sample_count < kMovingAverageWindow){
    ++m_sample_count;
  }
}

void Dht22Sensor::computeMovingAverage(float& temperature_c, float& humidity_percent) const {
  if(m_sample_count == 0){
    temperature_c = 0.0F;
    humidity_percent = 0.0F;
    return;
  }

  float temp_sum = 0.0F;
  float humidity_sum = 0.0F;
  for(uint8_t i = 0; i < m_sample_count; ++i){
    temp_sum += m_temp_samples[i];
    humidity_sum += m_humidity_samples[i];
  }

  const float count = static_cast<float>(m_sample_count);
  temperature_c = temp_sum / count;
  humidity_percent = humidity_sum / count;
}

void Dht22Sensor::publishState(uint32_t now_ms) const {
  const bool valid = m_is_valid && m_has_filtered;
  const float temperature_c =
      valid ? quantizeStep(m_filtered_temp_c, kTemperatureQuantizeC) : 0.0F;
  const float humidity_percent =
      valid ? quantizeStep(m_filtered_humidity_percent, kHumidityQuantizePercent) : 0.0F;

  m_state.setRoomClimateState(temperature_c, humidity_percent, valid, now_ms);
}

float Dht22Sensor::quantizeStep(float value, float step) {
  if(step <= 0.0F){
    return value;
  }
  return roundf(value / step) * step;
}

#include "sensors/Max31855Sensor.h"
#include "pins_arduino.h"

#include <math.h>

#include "pins.h"
// #include "variant_PILL_F103Cx.h"

namespace {
constexpr uint8_t kChipFaultMask = 0x07;
constexpr uint8_t kChipFaultCheckMask = 0x07;
}

Max31855Sensor::Max31855Sensor(SystemState& state)
    : m_state(state),
      m_spi(pinNametoDigitalPin(pin::MAX_CS), 
            pinNametoDigitalPin(pin::MAX_CLK), 
            pinNametoDigitalPin(pin::MAX_DO),
            -1, 1000000) 
{
}

void Max31855Sensor::setTracingEnabled(bool enabled) {
  m_tracing_enabled = enabled;
}

void Max31855Sensor::begin() {
  m_is_started = m_spi.begin();
  m_last_poll_ms = millis();
  m_is_valid = false;
  m_fault_flags = 0;
  m_fault_streak = 0;
  m_recover_streak = 0;
  m_outlier_streak = 0;
  m_plateau_ticks = 0;
  m_has_filtered = false;
  m_has_rate_reference = false;
  m_rate_c_per_s = 0.0F;
  m_prev_accepted_raw_c = 0.0F;
  m_prev_raw_c = 0.0F;

  if(!m_is_started){
    m_fault_flags = kFaultRange;
    Serial.println(F("MAX31855 init failed"));
  }

  publishState(m_last_poll_ms);
}

void Max31855Sensor::tick() {
  if(!m_is_started){
    return;
  }

  const uint32_t now_ms = millis();
  if((now_ms - m_last_poll_ms) < kPollIntervalMs){
    return;
  }
  m_last_poll_ms = now_ms;

  Sample sample;
  if(!readSample(sample)){
    return;
  }

  float raw_c = sample.raw_c;
  uint8_t fault_flags = sample.fault_flags;

  if(isnan(raw_c) || raw_c < kValidMinTempC || raw_c > kValidMaxTempC){
    fault_flags = static_cast<uint8_t>(fault_flags | kFaultRange);
  }

  if(fault_flags != 0){
    const float rate_for_trace = m_rate_c_per_s;
    m_fault_flags = fault_flags;
    m_recover_streak = 0;
    m_plateau_ticks = 0;
    if(m_fault_streak < 255){
      ++m_fault_streak;
    }

    if(m_fault_streak >= kFaultStrikes){
      m_is_valid = false;
      m_rate_c_per_s = 0.0F;
      m_has_rate_reference = false;
    }

    publishState(now_ms);
    traceSample(raw_c, rate_for_trace);
    return;
  }

  m_fault_streak = 0;
  m_fault_flags = 0;
  if(m_recover_streak < 255){
    ++m_recover_streak;
  }

  if(!m_is_valid){
    if(m_recover_streak < kRecoverStrikes){
      publishState(now_ms);
      traceSample(raw_c, m_rate_c_per_s);
      return;
    }

    m_is_valid = true;
    m_has_filtered = false;
    m_has_rate_reference = false;
    m_rate_c_per_s = 0.0F;
    m_plateau_ticks = 0;
  }

  const bool was_accepted = updateFilteredTemperature(raw_c);
  if(was_accepted){
    tryPlateauSnap(raw_c);
    updateTemperatureRate(now_ms);
  }else{
    m_plateau_ticks = 0;
  }

  publishState(now_ms);
  traceSample(raw_c, m_is_valid ? m_rate_c_per_s : 0.0F);
}

bool Max31855Sensor::readSample(Sample& sample) {
  uint8_t buf[4] = {0, 0, 0, 0};
  if(!m_spi.read(buf, 4)){
    sample.word = 0;
    sample.fault_flags = kChipFaultMask;
    sample.raw_c = NAN;
    return true;
  }

  uint32_t word = static_cast<uint32_t>(buf[0]) << 24;
  word |= static_cast<uint32_t>(buf[1]) << 16;
  word |= static_cast<uint32_t>(buf[2]) << 8;
  word |= static_cast<uint32_t>(buf[3]);

  sample.word = word;
  sample.fault_flags = static_cast<uint8_t>(word & kChipFaultMask);
  sample.raw_c = decodeCelsiusFromWord(word);
  return true;
}

float Max31855Sensor::decodeCelsiusFromWord(uint32_t word) {
  if(word & kChipFaultCheckMask){
    return NAN;
  }

  int32_t value = static_cast<int32_t>(word);
  if(word & 0x80000000U){
    value = static_cast<int32_t>(0xFFFFC000 | ((word >> 18) & 0x00003FFFU));
  }else{
    value = static_cast<int32_t>(word >> 18);
  }

  return static_cast<float>(value) * 0.25F;
}

void Max31855Sensor::publishState(uint32_t now_ms) const {
  const float temperature_c = m_has_filtered ? m_filtered_c : 0.0F;
  const float temp_rate_c_per_s = m_is_valid ? m_rate_c_per_s : 0.0F;

  m_state.setHeatExchangerState(temperature_c,
                                temp_rate_c_per_s,
                                m_is_valid,
                                m_fault_flags,
                                now_ms);
}

bool Max31855Sensor::updateFilteredTemperature(float raw_c) {
  if(!m_has_filtered){
    m_filtered_c = raw_c;
    m_has_filtered = true;
    m_prev_accepted_raw_c = raw_c;
    m_outlier_streak = 0;
    return true;
  }

  const float step_c = fabsf(raw_c - m_prev_accepted_raw_c);
  if(step_c > kMaxRawStepC){
    if(m_outlier_streak < 255){
      ++m_outlier_streak;
    }
    if(m_outlier_streak < kOutlierAcceptStrikes){
      return false;
    }
  }

  m_outlier_streak = 0;
  m_prev_accepted_raw_c = raw_c;
  m_filtered_c = (kFilterAlpha * raw_c) + ((1.0F - kFilterAlpha) * m_filtered_c);
  return true;
}

void Max31855Sensor::tryPlateauSnap(float raw_c) {
  if(!m_has_filtered || !m_is_valid){
    m_plateau_ticks = 0;
    m_prev_raw_c = raw_c;
    return;
  }

  if(fabsf(raw_c - m_filtered_c) < kPlateauSnapDeltaC &&
     fabsf(raw_c - m_prev_raw_c) < kPlateauRawStableC){
    if(m_plateau_ticks < 255){
      ++m_plateau_ticks;
    }
  }else{
    m_plateau_ticks = 0;
  }
  m_prev_raw_c = raw_c;

  if(m_plateau_ticks < kPlateauStableTicks){
    return;
  }

  m_filtered_c = raw_c;
  m_prev_rate_temp_c = m_filtered_c;
  m_rate_c_per_s = 0.0F;
  m_plateau_ticks = 0;
}

void Max31855Sensor::updateTemperatureRate(uint32_t now_ms) {
  if(!m_has_rate_reference){
    m_prev_rate_temp_c = m_filtered_c;
    m_prev_rate_ts_ms = now_ms;
    m_has_rate_reference = true;
    m_rate_c_per_s = 0.0F;
    return;
  }

  const uint32_t delta_ms = now_ms - m_prev_rate_ts_ms;
  if(delta_ms == 0){
    return;
  }

  const float dt_s = static_cast<float>(delta_ms) / 1000.0F;
  const float rate_raw = (m_filtered_c - m_prev_rate_temp_c) / dt_s;
  const float rate_clamped = clampFloat(rate_raw, -kRateLimitCPerS, kRateLimitCPerS);
  m_rate_c_per_s = (kRateBeta * rate_clamped) + ((1.0F - kRateBeta) * m_rate_c_per_s);

  m_prev_rate_temp_c = m_filtered_c;
  m_prev_rate_ts_ms = now_ms;
}

void Max31855Sensor::traceSample(float raw_c, float rate_for_trace) const {
  if(!m_tracing_enabled){
    return;
  }

  Serial.print(F("[MAX31855] raw="));
  if(isnan(raw_c)){
    Serial.print(F("nan"));
  }else{
    Serial.print(raw_c, 2);
  }
  Serial.print(F(" filt="));
  Serial.print(m_has_filtered ? m_filtered_c : 0.0F, 2);
  Serial.print(F(" rate="));
  Serial.print(rate_for_trace, 2);
  Serial.print(F(" valid="));
  Serial.print(m_is_valid ? 1 : 0);
  Serial.print(F(" fault=0x"));
  Serial.println(m_fault_flags, HEX);
}

float Max31855Sensor::clampFloat(float value, float min_value, float max_value) {
  if(value < min_value){
    return min_value;
  }
  if(value > max_value){
    return max_value;
  }
  return value;
}

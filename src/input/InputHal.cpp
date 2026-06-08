#include "input/InputHal.h"

#include <Arduino.h>

#include "pins.h"
#include "pins_arduino.h"

InputHal* InputHal::s_instance = nullptr;

namespace {

// Время стабилизации АЦП перед распознаванием кнопки.
constexpr uint32_t ADC_STABLE_MS = 30;

// Полуширина окна допуска: половина расстояния до ближайшего соседнего уровня.
uint16_t computeWindow(uint16_t level, const uint16_t* levels, uint8_t count, uint16_t max_window) {
  uint16_t min_dist = 4095;
  for (uint8_t i = 0; i < count; ++i) {
    if (levels[i] == level) {
      continue;
    }
    const uint16_t d = (levels[i] > level) ? (levels[i] - level) : (level - levels[i]);
    if (d < min_dist) {
      min_dist = d;
    }
  }
  const uint16_t half = static_cast<uint16_t>(min_dist / 2);
  return (half > max_window) ? max_window : half;
}

}  // namespace

void InputHal::begin(const ConfigManager::Config& cal) {
  s_instance = this;
  m_head = 0;
  m_tail = 0;
  m_stable_kbd = -1;
  m_last_adc = 0;
  m_adc_stable_ms = 0;
  m_adc_stable = false;
  m_calibration_mode = false;

  m_enc.init(pin::ENC_S1, pin::ENC_S2, pin::ENC_KEY, INPUT, INPUT_PULLUP, LOW);
  m_enc.setEncISR(true);
  m_enc.setHoldTimeout(1000);  // длинное нажатие оси → главное меню

  m_kbd[0].setHoldTimeout(1800);  // Power: аварийный стоп
  for (uint8_t i = 1; i < KBD_COUNT; ++i) {
    m_kbd[i].setHoldTimeout(600);
  }

  reloadCalibration(cal);

  attachInterrupt(digitalPinToInterrupt(pin::ENC_S1), InputHal::encIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin::ENC_S2), InputHal::encIsr, CHANGE);
}

void InputHal::reloadCalibration(const ConfigManager::Config& cal) {
  m_adc_level[0] = cal.btnPower;
  m_adc_level[1] = cal.btnUp;
  m_adc_level[2] = cal.btnIgnition;
  m_adc_level[3] = cal.btnPump;
  m_adc_level[4] = cal.btnDown;

  m_kbd_enabled = (cal.btnPower != 0);

  for (uint8_t i = 0; i < KBD_COUNT; ++i) {
    m_adc_window[i] = computeWindow(m_adc_level[i], m_adc_level, KBD_COUNT, ADC_MAX_WINDOW);
    if (m_adc_window[i] < 8) {
      m_adc_window[i] = 8;
    }
  }
}

void InputHal::setCalibrationMode(bool enabled) {
  m_calibration_mode = enabled;
}

uint16_t InputHal::getStableAdc() const {
  return m_last_adc;
}

bool InputHal::isKeyPressed() const {
  return m_adc_stable && (m_last_adc < ADC_IDLE_THRESHOLD);
}

void InputHal::tick() {
  tickKeyboard();
  tickEncoder();
}

bool InputHal::pop(RawInputEvent& out) {
  if (m_head == m_tail) {
    return false;
  }
  out = m_queue[m_tail];
  m_tail = static_cast<uint8_t>((m_tail + 1) % QUEUE_CAP);
  return true;
}

void InputHal::push(const RawInputEvent& ev) {
  const uint8_t next = static_cast<uint8_t>((m_head + 1) % QUEUE_CAP);
  if (next == m_tail) {
    return;  // очередь переполнена — событие отбрасывается
  }
  m_queue[m_head] = ev;
  m_head = next;
}

PhysicalButton InputHal::kbdIndexToPhysical(uint8_t index) {
  switch (index) {
    case 0:
      return PhysicalButton::Power;
    case 1:
      return PhysicalButton::Up;
    case 2:
      return PhysicalButton::Ignitor;
    case 3:
      return PhysicalButton::Pump;
    case 4:
      return PhysicalButton::Down;
    default:
      return PhysicalButton::None;
  }
}

int8_t InputHal::classifyAdc(uint16_t sample) const {
  int8_t best = -1;
  uint16_t best_dist = 4096;

  for (uint8_t i = 0; i < KBD_COUNT; ++i) {
    const uint16_t level = m_adc_level[i];
    const uint16_t window = m_adc_window[i];
    const uint16_t dist = (sample > level) ? (sample - level) : (level - sample);
    if (dist <= window && dist < best_dist) {
      best_dist = dist;
      best = static_cast<int8_t>(i);
    }
  }
  return best;
}

void InputHal::sampleKeyboardAdc() {
  const uint16_t sample = static_cast<uint16_t>(analogRead(aPin::KBD_OUT));
  const uint32_t now = millis();

  if (abs(static_cast<int32_t>(sample) - static_cast<int32_t>(m_last_adc)) > 8) {
    m_last_adc = sample;
    m_adc_stable_ms = now;
    m_adc_stable = false;
  } else if (!m_adc_stable && (now - m_adc_stable_ms) >= ADC_STABLE_MS) {
    m_adc_stable = true;
  }
}

void InputHal::tickKeyboard() {
  if (!m_kbd_enabled && !m_calibration_mode) {
    return;
  }

  sampleKeyboardAdc();

  if (m_calibration_mode) {
    return;
  }

  const uint32_t now = millis();
  int8_t active = -1;
  if (m_adc_stable) {
    active = classifyAdc(m_last_adc);
  }
  m_stable_kbd = active;

  // Каждая VirtButton получает «нажата / отпущена»; click/hold → в очередь.
  for (uint8_t i = 0; i < KBD_COUNT; ++i) {
    const bool down = (active == static_cast<int8_t>(i));
    m_kbd[i].tick(down);

    if (!m_kbd[i].click() && !m_kbd[i].hold()) {
      continue;
    }

    RawInputEvent ev{};
    ev.kind = RawInputEvent::Kind::Button;
    ev.timestamp_ms = now;
    ev.button.btn = kbdIndexToPhysical(i);
    ev.button.gesture = m_kbd[i].hold() ? ButtonGesture::LongPress : ButtonGesture::Click;
    push(ev);
  }
}

void InputHal::tickEncoder() {
  m_enc.tick();

  const uint32_t now = millis();

  if (m_enc.right()) {
    RawInputEvent ev{};
    ev.kind = RawInputEvent::Kind::Encoder;
    ev.timestamp_ms = now;
    ev.encoder.motion = EncoderMotion::StepCw;
    ev.encoder.steps = -1;
    push(ev);
  }
  if (m_enc.left()) {
    RawInputEvent ev{};
    ev.kind = RawInputEvent::Kind::Encoder;
    ev.timestamp_ms = now;
    ev.encoder.motion = EncoderMotion::StepCcw;
    ev.encoder.steps = 1;
    push(ev);
  }
  if (m_enc.click()) {
    RawInputEvent ev{};
    ev.kind = RawInputEvent::Kind::Encoder;
    ev.timestamp_ms = now;
    ev.encoder.motion = EncoderMotion::KeyClick;
    ev.encoder.steps = 0;
    push(ev);
  }
  if (m_enc.hold()) {
    RawInputEvent ev{};
    ev.kind = RawInputEvent::Kind::Encoder;
    ev.timestamp_ms = now;
    ev.encoder.motion = EncoderMotion::KeyLongPress;
    ev.encoder.steps = 0;
    push(ev);
  }
}

void InputHal::encIsr() {
  if (s_instance != nullptr) {
    s_instance->m_enc.tickISR();
  }
}

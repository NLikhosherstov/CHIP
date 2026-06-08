#pragma once

#include <EncButton.h>

#include "input/InputTypes.h"
#include "system/ConfigManager.h"

// Низкоуровневый ввод: Gyver EncButton (энкодер + ISR) и АЦП-клавиатура (5× VirtButton).
// Публикует RawInputEvent в кольцевую очередь для InputController.
class InputHal {
public:
  static constexpr size_t QUEUE_CAP = 16;       // глубина очереди событий
  static constexpr uint8_t KBD_COUNT = 5;       // Power, Up, Ignitor, Pump, Down
  static constexpr uint16_t ADC_MAX_WINDOW = 120;  // макс. полуширина окна АЦП
  static constexpr uint16_t ADC_IDLE_THRESHOLD = 4000;

  void begin(const ConfigManager::Config& cal);
  void reloadCalibration(const ConfigManager::Config& cal);
  void setCalibrationMode(bool enabled);
  void tick();

  uint16_t getStableAdc() const;
  bool isKeyPressed() const;

  bool pop(RawInputEvent& out);

private:
  void push(const RawInputEvent& ev);
  int8_t classifyAdc(uint16_t sample) const;
  void sampleKeyboardAdc();
  void tickKeyboard();
  void tickEncoder();
  static PhysicalButton kbdIndexToPhysical(uint8_t index);

  EncButton m_enc;
  VirtButton m_kbd[KBD_COUNT];

  uint16_t m_adc_level[KBD_COUNT];   // калибровочные уровни из Config
  uint16_t m_adc_window[KBD_COUNT]; // допуск ± от соседних кнопок
  bool m_kbd_enabled;                // false, если btnPower == 0
  bool m_calibration_mode;

  uint16_t m_last_adc;
  uint32_t m_adc_stable_ms;
  int8_t m_stable_kbd;
  bool m_adc_stable;

  RawInputEvent m_queue[QUEUE_CAP];
  uint8_t m_head;
  uint8_t m_tail;

  static InputHal* s_instance;
  static void encIsr();
};

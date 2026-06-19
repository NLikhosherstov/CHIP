#pragma once

#include <stdint.h>

// Физические кнопки резистивной клавиатуры (после декодирования АЦП).
enum class PhysicalButton : uint8_t {
  None = 0,
  Power,    // ON/OFF
  Up,       // Вверх
  Down,     // Вниз
  Pump,     // Насос
  Ignitor,  // Свеча
};

// Жест по кнопке: короткое или длинное нажатие.
enum class ButtonGesture : uint8_t {
  Click,
  LongPress,
};

// События энкодера (вращение и кнопка на оси).
enum class EncoderMotion : uint8_t {
  StepCw,        // шаг по часовой
  StepCcw,       // шаг против часовой
  KeyClick,      // клик по оси энкодера
  KeyLongPress,  // удержание оси (вход в главное меню)
};

// Сырое событие с нижнего уровня ввода (очередь InputHal → InputController).
struct RawInputEvent {
  enum class Kind : uint8_t { Button, Encoder } kind;
  uint32_t timestamp_ms;

  union {
    struct {
      PhysicalButton btn;
      ButtonGesture gesture;
    } button;
    struct {
      EncoderMotion motion;
      int8_t steps;  // для StepCw/Ccw: +1 / -1
    } encoder;
  };
};

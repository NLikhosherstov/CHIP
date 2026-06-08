#pragma once

#include <stdint.h>

// Семантические жесты UI: DisplayManager интерпретирует их по контексту экрана.
enum class UiInput : uint8_t {
  Power,      // кнопка ON/OFF (в главном меню — выход без сохранения)
  Up,         // кнопка «Вверх»
  Down,       // кнопка «Вниз»
  Left,       // кнопка «Насос» (на HOME — исполнитель; в меню — UI)
  Right,      // кнопка «Свеча»
  Click,      // клик энкодера
  LongClick,  // долгий клик энкодера
  Delta,      // поворот энкодера; payload: +1 / -1

  OpenQuickMenu,  // payload: индекс пункта по умолчанию (внутренний переход)
  CloseQuickMenu,
  OpenMainMenu,   // длинное нажатие энкодера или пункт «Меню»
};

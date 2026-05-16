#pragma once
#include <stdint.h>

// Заголовок иконок-битмапов.
// Иконки хранятся как 1-битные горизонтальные bitmap в PROGMEM-совместимом формате.
// Пока реальные данные не добавлены — все указатели равны nullptr.
// Вызывающий код обязан проверять != nullptr перед drawBitmap().
//
// Как заполнить данные:
//   1. Экспортировать SVG из Figma: выбрать узел иконки → Export → SVG.
//   2. Отрендерить в PNG нужного размера (Inkscape: File > Export PNG, указать w/h).
//   3. Конвертировать PNG в C-массив через https://javl.github.io/image2cpp/
//      Настройки: 1 bit per pixel, Horizontal, Draw mode: Horizontal.
//   4. Вставить массив в icons.cpp, заменив nullptr на имя массива.

// ─── Иконка термометра (температура воздуха) ──────────────────────────────────
extern const uint8_t* const ICON_TEMP_LOW;
static constexpr uint8_t ICON_TEMP_LOW_W = 24;
static constexpr uint8_t ICON_TEMP_LOW_H = 24;

// ─── Иконка капли с процентом (влажность) ─────────────────────────────────────
extern const uint8_t* const ICON_DROPLET_PCT;
static constexpr uint8_t ICON_DROPLET_W = 18;
static constexpr uint8_t ICON_DROPLET_H = 25;

// ─── Иконка насоса (двухслойная: внешний контур + внутренняя деталь) ──────────
extern const uint8_t* const ICON_PUMP_OUTER;
static constexpr uint8_t ICON_PUMP_OUTER_W = 23;
static constexpr uint8_t ICON_PUMP_OUTER_H = 32;

extern const uint8_t* const ICON_PUMP_INNER;
static constexpr uint8_t ICON_PUMP_INNER_W = 10;
static constexpr uint8_t ICON_PUMP_INNER_H = 10;

// ─── Иконка свечи зажигания (молния) ──────────────────────────────────────────
extern const uint8_t* const ICON_IGNITION;
static constexpr uint8_t ICON_IGNITION_W = 23;
static constexpr uint8_t ICON_IGNITION_H = 32;

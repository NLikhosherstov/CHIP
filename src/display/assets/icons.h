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

namespace icon{
    static constexpr uint16_t INDICATOR_ICON_WIDTH  = 32;
    static constexpr uint16_t INDICATOR_ICON_HEIGHT = 44;

    static constexpr uint16_t CORE_ICON_WIDTH  = 24;
    static constexpr uint16_t CORE_ICON_HEIGHT = 24;

    // Иконка термометра (температура воздуха) ─────────────────────────
    extern const uint8_t* const TEMP_PRIMARY;
    extern const uint8_t *const TEMP_SECONDARY;

    static constexpr uint8_t TEMP_W = CORE_ICON_WIDTH;
    static constexpr uint8_t TEMP_H = CORE_ICON_HEIGHT;

    // Иконка капли с процентом (влажность) ────────────────────────────
    extern const uint8_t* const DROPLET_PCT_PRIMARY;
    extern const uint8_t* const DROPLET_PCT_SECONDARY;

    static constexpr uint8_t DROPLET_W = CORE_ICON_WIDTH;
    static constexpr uint8_t DROPLET_H = CORE_ICON_WIDTH;

    // Иконка насоса ───────────────────────────────────────────────────
    extern const uint8_t* const PUMP_PRIMARY;
    extern const uint8_t* const PUMP_SECONDARY;

    static constexpr uint8_t PUMP_W = INDICATOR_ICON_WIDTH;
    static constexpr uint8_t PUMP_H = INDICATOR_ICON_HEIGHT;

    // Иконка свечи зажигания ──────────────────────────────────────────
    extern const uint8_t* const IGNITION_PRIMARY;
    extern const uint8_t* const IGNITION_SECONDARY;

    static constexpr uint8_t IGNITION_W = INDICATOR_ICON_WIDTH;
    static constexpr uint8_t IGNITION_H = INDICATOR_ICON_HEIGHT;
}
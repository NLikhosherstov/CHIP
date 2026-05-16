#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Насос» — нижний левый угол + иконка на левой боковой стороне.
//
// Блок значений (из Figma): x=9, y=194, 85x35px
//   Верхняя строка: "л/ч" — подпись единиц измерения
//   Нижняя строка:  расход топлива, Aldrich 24px
//
// Иконка насоса (боковая, по центру по вертикали): center_x≈18, center_y=119
struct PumpWidget {
    // Блок значения (нижний левый угол)
    static constexpr int16_t X = 9,  Y = 194;
    static constexpr int16_t W = 85, H = 35;

    // Иконка (боковая, левее кольца Core)
    static constexpr int16_t ICON_CX = 18;
    static constexpr int16_t ICON_CY = 119;
    static constexpr int16_t ICON_W  = 23;
    static constexpr int16_t ICON_H  = 32;

    // pulseHz — частота импульсов насоса (pulse_hz из PumpState), используется для расчёта л/ч.
    // Для отображения преобразуем: 1 импульс ≈ 0.015 мл (TODO: уточнить калибровку).
    static void draw        (TFT_eSPI& tft, uint16_t pulseHz, bool active, const PaletteRGB565& pal);
    static void updateFlow  (TFT_eSPI& tft, uint16_t pulseHz,             const PaletteRGB565& pal);
    static void drawIcon    (TFT_eSPI& tft,                  bool active, const PaletteRGB565& pal);

private:
    // Перевод частоты импульсов насоса в условные единицы для отображения.
    // Формат строки: "X.XX" (четыре символа)
    static void formatFlow(char* buf, uint8_t bufSize, uint16_t pulseHz);
};

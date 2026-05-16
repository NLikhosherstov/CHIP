#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Скорость вентилятора» — верхний правый угол.
// Отображает 4 прямоугольных сегмента + цифру выбранной ступени.
//
// Позиция (из Figma): x=224, y=9, 85x36px
// Сегменты: 4 шт., каждый 14x20px, зазор 2px, правовыравнены.
// Цифра ступени: Aldrich 28px, справа от сегментов.
struct SpeedWidget {
    static constexpr int16_t X = 224, Y = 9;
    static constexpr int16_t W = 85,  H = 36;

    static constexpr uint8_t  BAR_COUNT  = 4;
    static constexpr int16_t  BAR_W      = 14;
    static constexpr int16_t  BAR_H      = 20;
    static constexpr int16_t  BAR_GAP    = 2;

    // speedStep: 0 = вентилятор выключен, 1..4 = активная ступень
    static void draw      (TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal);
    static void updateStep(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal);

private:
    static void drawBars  (TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal);
    static void drawLabel (TFT_eSPI& tft,                    const PaletteRGB565& pal);
};

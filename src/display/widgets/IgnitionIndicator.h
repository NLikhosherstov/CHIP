#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Индикатор свечи зажигания» — правая боковая сторона экрана.
// Иконка-молния (FontAwesome bolt), центрирована по вертикали.
//
// Позиция (из Figma): center_x=289, center_y=119, 23x32px
struct IgnitionIndicator {
    static constexpr int16_t CX = 289;
    static constexpr int16_t CY = 119;
    static constexpr int16_t W  = 23;
    static constexpr int16_t H  = 32;

    static void draw(TFT_eSPI& tft, bool active, const PaletteRGB565& pal);
};

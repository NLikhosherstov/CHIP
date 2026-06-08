#pragma once

#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Боковой индикатор насоса (левая сторона экрана), 32×44 px.
struct PumpIndicator {
    static constexpr int16_t X = 2;
    static constexpr int16_t Y = 98;

    static void draw(TFT_eSPI& tft, bool active, const PaletteRGB565& pal);
};

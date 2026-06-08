#pragma once

#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Индикатор свечи зажигания — правая боковая сторона, 32×44 px.
struct IgnitionIndicator {
    static constexpr int16_t X = 272;
    static constexpr int16_t Y = 98;

    static void draw(TFT_eSPI& tft, bool active, const PaletteRGB565& pal);
};

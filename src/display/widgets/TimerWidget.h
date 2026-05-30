#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

struct TimerWidget {
    static constexpr int16_t X = 240, Y = 201;
    static constexpr int16_t W = 71,  H = 36;

    static constexpr int16_t VALUES_PUSH_X = X + W - 85;
    static constexpr int16_t VALUES_PUSH_Y = Y + 12;

    static void draw(TFT_eSPI& tft, uint16_t elapsedSec, uint16_t maxSec, const PaletteRGB565& pal);
    static void update(TFT_eSPI& tft, uint16_t elapsedSec, uint16_t maxSec, const PaletteRGB565& pal);
};

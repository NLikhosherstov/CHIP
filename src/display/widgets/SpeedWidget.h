#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

struct SpeedWidget {
    static constexpr int16_t X = 227, Y = 8;
    static constexpr int16_t W = 85,  H = 36;

    static constexpr uint8_t  BAR_COUNT  = 4;
    static constexpr int16_t  BAR_W      = 12;
    static constexpr int16_t  BAR_H      = 20;
    static constexpr int16_t  BAR_GAP    = 3;

    static constexpr int16_t DYNAMIC_PUSH_X = X;
    static constexpr int16_t DYNAMIC_PUSH_Y = Y;

    static void draw(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal);
    static void updateStep(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal);

private:
    static void drawBars(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal);
    static void drawLabel(TFT_eSPI& tft, const PaletteRGB565& pal);
};

#pragma once

#include <stdint.h>
#include <stddef.h>

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Насос» — нижний левый угол (подпись + расход). Иконка — PumpIndicator.
struct PumpWidget {
    static constexpr int16_t X = 4,  Y = 202;
    static constexpr int16_t W = 85, H = 35;

    static constexpr int16_t FLOW_PUSH_X = X;
    static constexpr int16_t FLOW_PUSH_Y = Y + 14;

    static void draw(TFT_eSPI& tft,
                     uint16_t cycle_period_ms,
                     uint16_t pump_performance,
                     bool active,
                     const PaletteRGB565& pal);
    static void updateFlow(TFT_eSPI& tft,
                           uint16_t cycle_period_ms,
                           uint16_t pump_performance,
                           const PaletteRGB565& pal);

private:
    static void formatFlow(char* buf,
                           size_t bufSize,
                           uint16_t cycle_period_ms,
                           uint16_t pump_performance);

};

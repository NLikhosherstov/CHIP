#pragma once
#include <stdint.h>
#include "system/SystemState.h"

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Режим» — верхний левый угол.
// Отображает название текущего режима и подпись "режим".
struct ModeWidget {
    static constexpr int16_t X = 4,  Y = 4;
    static constexpr int16_t W = 100, H = 19;

    static void draw  (TFT_eSPI& tft, SystemState::AutomationState state, const PaletteRGB565& pal);
    static void update(TFT_eSPI& tft, SystemState::AutomationState state, const PaletteRGB565& pal);

private:
    static const char* modeName(SystemState::AutomationState state);
};

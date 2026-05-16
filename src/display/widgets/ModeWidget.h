#pragma once
#include <stdint.h>
#include "system/SystemState.h"

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Режим» — верхний левый угол.
// Отображает название текущего режима и подпись "режим".
//
// Позиция (из Figma): x=9, y=9, 85x37px
struct ModeWidget {
    static constexpr int16_t X = 9,  Y = 9;
    static constexpr int16_t W = 85, H = 37;

    static void draw  (TFT_eSPI& tft, SystemState::AutomationState state, const PaletteRGB565& pal);
    static void update(TFT_eSPI& tft, SystemState::AutomationState state, const PaletteRGB565& pal);

private:
    static const char* modeName(SystemState::AutomationState state);
};

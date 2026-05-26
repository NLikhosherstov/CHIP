#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Таймер» — нижний правый угол.
struct TimerWidget {
    static constexpr int16_t X = 240, Y = 201;
    static constexpr int16_t W = 71,  H = 36;

    // elapsedSec — прошедшие секунды (0 если свеча выключена)
    // maxSec     — максимальное время из ConfigManager::Config::ignitor_timeout_s
    static void draw  (TFT_eSPI& tft, uint16_t elapsedSec, uint16_t maxSec, const PaletteRGB565& pal);
    static void update(TFT_eSPI& tft, uint16_t elapsedSec, uint16_t maxSec, const PaletteRGB565& pal);
};

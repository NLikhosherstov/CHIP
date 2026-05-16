#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Виджет «Таймер свечи» — нижний правый угол.
// Отображает секунды с момента включения свечи / максимальное время из конфига.
// Формат: "12/60" (текущее секунды ярко, "/60" затухшим цветом).
//
// Позиция (из Figma): правый край x=311 (x=226, w=85), y=196, 85x33px
struct TimerWidget {
    static constexpr int16_t X = 226, Y = 196;
    static constexpr int16_t W = 85,  H = 33;

    // elapsedSec — прошедшие секунды (0 если свеча выключена)
    // maxSec     — максимальное время из ConfigManager::Config::ignitor_timeout_s
    static void draw  (TFT_eSPI& tft, uint16_t elapsedSec, uint16_t maxSec, const PaletteRGB565& pal);
    static void update(TFT_eSPI& tft, uint16_t elapsedSec, uint16_t maxSec, const PaletteRGB565& pal);
};

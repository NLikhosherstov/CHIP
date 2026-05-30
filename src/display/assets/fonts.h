#pragma once

// Шрифтовые ресурсы.
//   Файл ttf должен содержать один стиль
//   Растрировать определенный диапазон в https://rop.nl/truetype2gfx/
//   Отредактировать глифы и выбрать диапазон символов в https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

#include <Arduino.h>
#include <Fonts/GFXFF/gfxfont.h>
#include <TFT_eSPI.h>

extern const GFXfont* const Font_h1;        // Температура теплообменника
extern const GFXfont* const Font_h2;        // Температура/влажность помещения
extern const GFXfont* const Font_h3;        // Цифра скорости
extern const GFXfont* const Font_default;   // Значения виджетов
extern const GFXfont* const Font_small;     // Мелкие подписи ("режим", "л/ч", "сек")
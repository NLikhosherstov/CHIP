#pragma once

// Шрифтовые ресурсы.
//   Файл ttf должен содержать один стиль
//   Растрировать определенный диапазон в https://rop.nl/truetype2gfx/
//   Отредактировать глифы и выбрать диапазон символов в https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace smooth_font{
    extern const uint8_t* const h1;        // Температура теплообменника
    extern const uint8_t* const h2;        // Температура/влажность помещения
    extern const uint8_t* const h3;        // Цифра скорости
    extern const uint8_t* const def;       // Значения виджетов
    extern const uint8_t* const small;     // Мелкие подписи ("режим", "л/ч", "сек")
}
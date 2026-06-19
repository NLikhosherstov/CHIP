#include "display/assets/fonts.h"

// ─── Загрузка шрифтов ─────────────────────────────────────────────────────────
// -> https://fonts.google.com/ 
// -> https://rop.nl/truetype2gfx/ 
// -> https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
// или
// -> Генерация name.vlw
// -> python tools\convert_font.py "path\name.vlw"
// -> python tools\convert_font.py "D:\Projects\ProcessingFonts\fontGen\default.vlw"

#include "h1.h"
#include "h2.h"
#include "h3.h"
#include "default.h"
#include "small.h"

namespace smooth_font{
    extern const uint8_t* const h1 = h1_vlw;        //0123456789-
    extern const uint8_t* const h2 = h2_vlw;        //0123456789
    extern const uint8_t* const h3 = h3_vlw;        //0123456789
    extern const uint8_t* const def = default_vlw;  //0123456789АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя -.,/°%
    extern const uint8_t* const small = small_vlw;  //0123456789АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя .,/-
}

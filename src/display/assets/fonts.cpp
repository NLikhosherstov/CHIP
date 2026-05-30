#include "display/assets/fonts.h"

// ─── Загрузка шрифтов ─────────────────────────────────────────────────────────
// -> https://fonts.google.com/ 
// -> https://rop.nl/truetype2gfx/ 
// -> https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

#include "Aldrich_Regular70pt7b.h"
#include "Aldrich_Regular34pt7b.h"
#include "Aldrich_Regular28pt7b.h"
#include "Roboto_Black12pt7b.h"
#include "RobotoMono_Bold12pt7b.h"

const GFXfont* const Font_h1        = &Aldrich_Regular36pt7b;
const GFXfont* const Font_h2        = &Aldrich_Regular18pt7b;
const GFXfont* const Font_h3        = &Aldrich_Regular15pt7b;
const GFXfont* const Font_default   = &Roboto_Black12pt7b;
const GFXfont* const Font_small     = &RobotoMono_Bold7pt7b;

const GFXfont* const Font_RobotoMono_40 = nullptr;

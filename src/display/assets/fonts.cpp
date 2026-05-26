#include "display/assets/fonts.h"

// ─── Заглушки шрифтов ─────────────────────────────────────────────────────────
// Все указатели установлены в nullptr.
// TFT_eSPI при nullptr в setFreeFont() использует встроенный шрифт (безопасно).
//
// Чтобы подключить реальный шрифт, замени nullptr на указатель на GFXfont-структуру:
//
//   #include "display/assets/font_aldrich_70.h"  // сгенерирован fontconvert
//   const GFXfont* const Font_Aldrich_70 = &Aldrich_70pt7b;
//
// Имена структур в сгенерированных файлах назначаются fontconvert автоматически.

#include "Aldrich_Regular70pt7b.h"
#include "Aldrich_Regular34pt7b.h"
#include "Aldrich_Regular28pt7b.h"
#include "Aldrich_Regular24pt7b.h"
// #include "RobotoMono_Bold40pt7b.h"
#include "RobotoMono_Bold12pt7b.h"

const GFXfont* const Font_Aldrich_70    = &Aldrich_Regular36pt7b;
const GFXfont* const Font_Aldrich_34    = &Aldrich_Regular18pt7b;
const GFXfont* const Font_Aldrich_28    = &Aldrich_Regular15pt7b;
const GFXfont* const Font_Aldrich_24    = &Aldrich_Regular12pt7b;
const GFXfont* const Font_RobotoMono_40 = nullptr;
const GFXfont* const Font_RobotoMono_12 = &RobotoMono_Bold7pt7b;

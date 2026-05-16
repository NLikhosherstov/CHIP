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

const GFXfont* const Font_Aldrich_70   = nullptr;
const GFXfont* const Font_Aldrich_34   = nullptr;
const GFXfont* const Font_Aldrich_28   = nullptr;
const GFXfont* const Font_Aldrich_24   = nullptr;
const GFXfont* const Font_RobotoMono_40 = nullptr;
const GFXfont* const Font_RobotoMono_12 = nullptr;

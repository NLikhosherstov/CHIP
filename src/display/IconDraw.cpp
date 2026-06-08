#include "display/IconDraw.h"

#include <TFT_eSPI.h>

namespace IconDraw {
namespace {

void drawGlyph(TFT_eSPI& dev,
               int16_t cx,
               int16_t cy,
               const uint8_t* font,
               const char* glyph,
               uint16_t color,
               uint16_t bg) {
    if (font == nullptr || glyph == nullptr || glyph[0] == '\0') {
        return;
    }

    dev.loadFont(font);
    dev.setTextDatum(MC_DATUM);
    dev.setTextColor(color, bg);
    dev.drawString(glyph, cx, cy);
    dev.unloadFont();
    dev.setTextFont(1);
}

}  // namespace

void drawIcon(TFT_eSPI& dev,
              int16_t cx,
              int16_t cy,
              const uint8_t* font,
              const char* glyph,
              uint16_t color,
              uint16_t bg) {
    drawGlyph(dev, cx, cy, font, glyph, color, bg);
}

void drawIcon(TFT_eSprite& spr,
              int16_t cx,
              int16_t cy,
              const uint8_t* font,
              const char* glyph,
              uint16_t color,
              uint16_t bg) {
    drawGlyph(spr, cx, cy, font, glyph, color, bg);
}

}  // namespace IconDraw

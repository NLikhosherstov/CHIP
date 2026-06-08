#pragma once

#include <stdint.h>

class TFT_eSPI;
class TFT_eSprite;

namespace IconDraw {

void drawIcon(TFT_eSPI& dev,
              int16_t cx,
              int16_t cy,
              const uint8_t* font,
              const char* glyph,
              uint16_t color,
              uint16_t bg);

void drawIcon(TFT_eSprite& spr,
              int16_t cx,
              int16_t cy,
              const uint8_t* font,
              const char* glyph,
              uint16_t color,
              uint16_t bg);

}  // namespace IconDraw

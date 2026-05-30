#pragma once

#include <stdint.h>

class TFT_eSPI;
class TFT_eSprite;

namespace IconDraw {
// Двухслойная 1-bit иконка: primary + secondary с alphaBlend(102, outer, bg).
void drawLayeredBitmap(TFT_eSPI& tft,
                       int16_t x,
                       int16_t y,
                       const uint8_t* primary,
                       const uint8_t* secondary,
                       uint8_t w,
                       uint8_t h,
                       uint16_t primaryColor,
                       uint16_t bg);
                       
void drawLayeredBitmap(TFT_eSPI& tft,
                       int16_t x,
                       int16_t y,
                       const uint8_t* primary,
                       const uint8_t* secondary,
                       uint8_t w,
                       uint8_t h,
                       uint16_t primaryColor);

void drawLayeredBitmap(TFT_eSprite& spr,
                       int16_t x,
                       int16_t y,
                       const uint8_t* primary,
                       const uint8_t* secondary,
                       uint8_t w,
                       uint8_t h,
                       uint16_t primaryColor,
                       uint16_t bg);

// Только маска «1»-пикселей, без заливки фона (фиксированные иконки).
void drawBitmapNoBg(TFT_eSPI& tft,
                    int16_t x,
                    int16_t y,
                    const uint8_t* bitmap,
                    uint8_t w,
                    uint8_t h,
                    uint16_t color);
}  // namespace IconDraw

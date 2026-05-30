#include "display/IconDraw.h"

#include <TFT_eSPI.h>

namespace IconDraw {
    namespace {
        constexpr uint8_t k_secondaryBlend = 102;

        void drawLayers(TFT_eSPI& dev,
                        int16_t x,
                        int16_t y,
                        const uint8_t* primary,
                        const uint8_t* secondary,
                        uint8_t w,
                        uint8_t h,
                        uint16_t primaryColor,
                        uint16_t bg) 
        {
            const uint16_t secondaryColor = dev.alphaBlend(k_secondaryBlend, primaryColor, bg);
            if (primary != nullptr) {
                dev.drawBitmap(x, y, primary, w, h, primaryColor);
            }
            if (secondary != nullptr) {
                dev.drawBitmap(x, y, secondary, w, h, secondaryColor);
            }
        }

        void drawLayersNoBg(TFT_eSPI& dev,
                            int16_t x,
                            int16_t y,
                            const uint8_t* primary,
                            const uint8_t* secondary,
                            uint8_t w,
                            uint8_t h,
                            uint16_t primaryColor) 
        {
            const uint16_t secondaryColor = dev.alphaBlend(k_secondaryBlend, TFT_BLACK, primaryColor);
            if (primary != nullptr) {
                dev.drawBitmap(x, y, primary, w, h, primaryColor);
            }
            if (secondary != nullptr) {
                dev.drawBitmap(x, y, secondary, w, h, secondaryColor);
            }
        }
    }  // namespace

void drawLayeredBitmap(TFT_eSPI& tft,
                       int16_t x,
                       int16_t y,
                       const uint8_t* primary,
                       const uint8_t* secondary,
                       uint8_t w,
                       uint8_t h,
                       uint16_t primaryColor,
                       uint16_t bg) {
    drawLayers(tft, x, y, primary, secondary, w, h, primaryColor, bg);
}

void drawLayeredBitmap(TFT_eSPI& tft,
                       int16_t x,
                       int16_t y,
                       const uint8_t* primary,
                       const uint8_t* secondary,
                       uint8_t w,
                       uint8_t h,
                       uint16_t primaryColor) {
    drawLayersNoBg(tft, x, y, primary, secondary, w, h, primaryColor);
}

void drawLayeredBitmap(TFT_eSprite& spr,
                       int16_t x,
                       int16_t y,
                       const uint8_t* primary,
                       const uint8_t* secondary,
                       uint8_t w,
                       uint8_t h,
                       uint16_t primaryColor,
                       uint16_t bg) {
    drawLayers(spr, x, y, primary, secondary, w, h, primaryColor, bg);
}

void drawBitmapNoBg(TFT_eSPI& tft,
                    int16_t x,
                    int16_t y,
                    const uint8_t* bitmap,
                    uint8_t w,
                    uint8_t h,
                    uint16_t color) {
    if (bitmap != nullptr) {
        tft.drawBitmap(x, y, bitmap, w, h, color);
    }
}
}  // namespace IconDraw

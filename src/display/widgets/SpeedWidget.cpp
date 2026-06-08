#include "display/widgets/SpeedWidget.h"

#include <TFT_eSPI.h>
#include <cstdint>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"

namespace {
char s_lastStepBuf[3] = "";

static const SpriteScreenRect kScreen = {SpeedWidget::DYNAMIC_PUSH_X, SpeedWidget::DYNAMIC_PUSH_Y,
                                         SpriteSlot::M};
}  // namespace

void SpeedWidget::draw(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    s_lastStepBuf[0] = '\0';
    drawLabel(tft, pal);
    drawBars(tft, speedStep, pal);
}

void SpeedWidget::updateStep(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(speedStep));

    const bool stepChanged = WidgetText::textChanged(buf, s_lastStepBuf, sizeof(s_lastStepBuf));

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::M);
    if (spr == nullptr) {
        if (stepChanged) {
            SpritePool::drawOomMarker(tft, kScreen);
        }
        return;
    }

    spr->fillSprite(pal.screenBg);

    const int16_t barsWidth = BAR_COUNT * BAR_W + (BAR_COUNT - 1) * BAR_GAP;
    for (uint8_t i = 0; i < BAR_COUNT; ++i) {
        const int16_t bx = static_cast<int16_t>(i) * (BAR_W + BAR_GAP);
        const bool filled = (i < speedStep);
        if(filled){
            spr->drawSmoothRoundRect(bx, SpritePool::M_H - BAR_H, 
                                     BAR_ROUND, BAR_ROUND - 1,         
                                     BAR_W, BAR_H-1,               
                                     pal.speedBarBorderColor, pal.screenBg);   
            spr->fillSmoothRoundRect(bx + 2, SpritePool::M_H - BAR_H + 2, 
                                     BAR_W - 3, BAR_H - 4, BAR_ROUND - 1, 
                                     pal.speedBarFillColor, pal.speedBarBorderColor);
        }else{
            spr->drawSmoothRoundRect(bx, SpritePool::M_H - BAR_H, 
                                     BAR_ROUND, BAR_ROUND - 1,         
                                     BAR_W, BAR_H-1,               
                                     pal.speedBarBorderColor, pal.screenBg);         
        }
    }

    if (stepChanged) {
        const int8_t charHalfWidth = 9;
        const int16_t digitX = barsWidth + BAR_GAP + charHalfWidth;
        spr->loadFont(smooth_font::h3);
        spr->setTextDatum(TC_DATUM);
        spr->setTextColor(pal.valueColor, pal.screenBg);
        spr->drawString(buf, digitX, 6);
        spr->unloadFont();
    }

    spr->pushSprite(kScreen.x, kScreen.y);
    SpritePool::release(SpriteSlot::M);
    tft.setTextFont(1);
}

void SpeedWidget::drawLabel(TFT_eSPI& tft, const PaletteRGB565& pal) {
    tft.loadFont(smooth_font::small);
    tft.setTextDatum(BR_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.startWrite();
    tft.drawString("СКОРОСТЬ", X + W, Y + H);
    tft.endWrite();
    tft.unloadFont();
    tft.setTextFont(1);
}

void SpeedWidget::drawBars(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    updateStep(tft, speedStep, pal);
}

#include "display/widgets/SpeedWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"

namespace {
char s_lastStepBuf[3] = "";

static const SpriteScreenRect kScreen = {SpeedWidget::DYNAMIC_PUSH_X, SpeedWidget::DYNAMIC_PUSH_Y,
                                         SpriteSlot::Medium};
}  // namespace

void SpeedWidget::draw(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    s_lastStepBuf[0] = '\0';
    tft.startWrite();
    tft.fillRect(X, Y, W, H, pal.screenBg);
    tft.endWrite();
    drawLabel(tft, pal);
    drawBars(tft, speedStep, pal);
}

void SpeedWidget::updateStep(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(speedStep));

    const bool stepChanged = WidgetText::textChanged(buf, s_lastStepBuf, sizeof(s_lastStepBuf));

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::Medium);
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
        spr->fillRect(bx, 0, BAR_W, BAR_H,
                      filled ? pal.speedBarFillColor : pal.speedBarEmptyColor);
    }

    if (stepChanged) {
        const int16_t digitX = barsWidth + BAR_GAP;
        spr->setFreeFont(Font_h3);
        spr->setTextDatum(ML_DATUM);
        spr->setTextColor(pal.valueColor, pal.screenBg);
        spr->drawString(buf, digitX, BAR_H / 2 - 1);
    }

    spr->pushSprite(kScreen.x, kScreen.y);
    SpritePool::release(SpriteSlot::Medium);
    tft.setTextFont(1);
}

void SpeedWidget::drawLabel(TFT_eSPI& tft, const PaletteRGB565& pal) {
    tft.setFreeFont(Font_small);
    tft.setTextDatum(BR_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.startWrite();
    tft.drawString("SPEED", X + W, Y + H);
    tft.endWrite();
    tft.setTextFont(1);
}

void SpeedWidget::drawBars(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    updateStep(tft, speedStep, pal);
}

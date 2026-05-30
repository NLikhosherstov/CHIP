#include "display/widgets/TimerWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"

namespace {
char s_lastCurBuf[6] = "";
char s_lastMaxBuf[8] = "";
bool s_labelDrawn    = false;

static const SpriteScreenRect kScreen = {TimerWidget::VALUES_PUSH_X, TimerWidget::VALUES_PUSH_Y,
                                         SpriteSlot::Medium};
}  // namespace

void TimerWidget::draw(TFT_eSPI& tft,
                       uint16_t elapsedSec,
                       uint16_t maxSec,
                       const PaletteRGB565& pal) {
    s_lastCurBuf[0] = '\0';
    s_lastMaxBuf[0] = '\0';
    s_labelDrawn     = false;

    tft.startWrite();
    tft.fillRect(X, Y, W, H, pal.screenBg);
    tft.endWrite();

    tft.setFreeFont(Font_small);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.startWrite();
    tft.drawString("TIMER", X + W, Y);
    tft.endWrite();
    tft.setTextFont(1);
    s_labelDrawn = true;

    update(tft, elapsedSec, maxSec, pal);
}

void TimerWidget::update(TFT_eSPI& tft,
                          uint16_t elapsedSec,
                          uint16_t maxSec,
                          const PaletteRGB565& pal) {
    char bufMax[8];
    snprintf(bufMax, sizeof(bufMax), "/%d", static_cast<int>(maxSec));

    char bufCur[6];
    snprintf(bufCur, sizeof(bufCur), "%d", static_cast<int>(elapsedSec));

    const bool curChanged = WidgetText::textChanged(bufCur, s_lastCurBuf, sizeof(s_lastCurBuf));
    const bool maxChanged = WidgetText::textChanged(bufMax, s_lastMaxBuf, sizeof(s_lastMaxBuf));

    if (!curChanged && !maxChanged) {
        return;
    }

    if (!s_labelDrawn) {
        tft.setFreeFont(Font_small);
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(pal.labelColor, pal.screenBg);
        tft.startWrite();
        tft.drawString("TIMER", X + W, Y);
        tft.endWrite();
        tft.setTextFont(1);
        s_labelDrawn = true;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::Medium);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kScreen);
        return;
    }

    spr->fillSprite(pal.screenBg);
    spr->setFreeFont(Font_default);

    const int16_t maxW    = spr->textWidth(bufMax);
    const int16_t spriteW = SpritePool::MEDIUM_W;
    const int16_t spriteH = SpritePool::MEDIUM_H;
    const int16_t baseY   = spriteH - 2;

    spr->setTextDatum(BR_DATUM);
    spr->setTextColor(pal.valueDimColor, pal.screenBg);
    spr->drawString(bufMax, spriteW, baseY);

    spr->setTextColor(pal.valueColor, pal.screenBg);
    spr->drawString(bufCur, spriteW - maxW, baseY);

    spr->pushSprite(kScreen.x, kScreen.y);
    SpritePool::release(SpriteSlot::Medium);
    tft.setTextFont(1);
}

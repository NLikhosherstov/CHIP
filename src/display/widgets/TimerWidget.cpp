#include "display/widgets/TimerWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"

namespace {
char s_lastCurBuf[6]  = "";
char s_lastMaxBuf[8]  = "";
bool s_labelDrawn     = false;

static constexpr int16_t VALUES_Y = TimerWidget::Y + 12;
static constexpr int16_t VALUES_H = 24;
}  // namespace

void TimerWidget::draw(TFT_eSPI& tft,
                       uint16_t elapsedSec,
                       uint16_t maxSec,
                       const PaletteRGB565& pal) {
    s_lastCurBuf[0] = '\0';
    s_lastMaxBuf[0] = '\0';
    s_labelDrawn = false;

    tft.startWrite();
    tft.fillRect(X, Y, W, H, pal.screenBg);
    tft.endWrite();

    tft.setFreeFont(Font_RobotoMono_12);
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
        tft.setFreeFont(Font_RobotoMono_12);
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(pal.labelColor, pal.screenBg);
        tft.startWrite();
        tft.drawString("TIMER", X + W, Y);
        tft.endWrite();
        tft.setTextFont(1);
        s_labelDrawn = true;
    }

    tft.setFreeFont(Font_Aldrich_24);
    const int16_t maxW = tft.textWidth(bufMax);
    const int16_t curW = tft.textWidth(bufCur);
    const int16_t clearW = maxW + curW + 4;
    const int16_t clearX = X + W - clearW;

    tft.startWrite();
    tft.fillRect(clearX, VALUES_Y, clearW, VALUES_H, pal.screenBg);

    tft.setTextDatum(BR_DATUM);
    tft.setTextColor(pal.valueDimColor, pal.screenBg);
    tft.drawString(bufMax, X + W, Y + H);

    tft.setTextColor(pal.valueColor, pal.screenBg);
    tft.drawString(bufCur, X + W - maxW, Y + H);
    tft.endWrite();

    tft.setTextFont(1);
}

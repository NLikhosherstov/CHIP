#include "display/widgets/SpeedWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"

namespace {
char s_lastStepBuf[3] = "";
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
    const int16_t barsLeft = X;
    const int16_t barsTop  = Y;
    const int16_t barsWidth = BAR_COUNT * BAR_W + (BAR_COUNT - 1) * BAR_GAP;

    tft.startWrite();
    tft.fillRect(barsLeft, barsTop, barsWidth, BAR_H, pal.screenBg);
    for (uint8_t i = 0; i < BAR_COUNT; ++i) {
        const int16_t bx = barsLeft + static_cast<int16_t>(i) * (BAR_W + BAR_GAP);
        const bool filled = (i < speedStep);
        tft.fillRect(bx, barsTop, BAR_W, BAR_H,
                     filled ? pal.speedBarFillColor : pal.speedBarEmptyColor);
    }
    tft.endWrite();

    char buf[3];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(speedStep));

    if (!WidgetText::textChanged(buf, s_lastStepBuf, sizeof(s_lastStepBuf))) {
        return;
    }

    const int16_t digitX = barsLeft + barsWidth + BAR_GAP;

    tft.setFreeFont(Font_Aldrich_28);
    const int16_t textW = tft.textWidth(buf);
    const int16_t clearX = digitX - 2;
    const int16_t clearW = textW + 4;

    tft.startWrite();
    tft.fillRect(clearX, barsTop, clearW, BAR_H, pal.screenBg);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(pal.valueColor, pal.screenBg);
    tft.drawString(buf, digitX, barsTop + BAR_H / 2 - 1);
    tft.endWrite();

    tft.setTextFont(1);
}

void SpeedWidget::drawLabel(TFT_eSPI& tft, const PaletteRGB565& pal) {
    tft.setFreeFont(Font_RobotoMono_12);
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

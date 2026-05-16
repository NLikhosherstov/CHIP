#include "display/widgets/SpeedWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"

void SpeedWidget::draw(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    tft.fillRect(X, Y, W, H, pal.screenBg);
    drawLabel(tft, pal);
    drawBars(tft, speedStep, pal);
}

void SpeedWidget::updateStep(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    // Перерисовываем только нижнюю строку с сегментами и цифрой
    tft.fillRect(X, Y + 14, W, H - 14, pal.screenBg);
    drawBars(tft, speedStep, pal);
}

void SpeedWidget::drawLabel(TFT_eSPI& tft, const PaletteRGB565& pal) {
    tft.setFreeFont(Font_RobotoMono_12);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.drawString("\xF1\xEA\xEE\xF0\xEE\xF1\xF2\xFC", X + W, Y); // "скорость"
    tft.setTextFont(1);
}

void SpeedWidget::drawBars(TFT_eSPI& tft, uint8_t speedStep, const PaletteRGB565& pal) {
    // Сегменты правовыравнены. Правый край = X + W.
    // Общая ширина сегментов: BAR_COUNT * BAR_W + (BAR_COUNT-1) * BAR_GAP = 62px
    // Цифра ступени занимает оставшееся место слева от сегментов.
    const int16_t barsRight  = X + W;                                       // 309
    const int16_t barsLeft   = barsRight - (BAR_COUNT * BAR_W + (BAR_COUNT - 1) * BAR_GAP); // 247
    const int16_t barsTop    = Y + (H - BAR_H) / 2 + 6;

    for (uint8_t i = 0; i < BAR_COUNT; ++i) {
        const int16_t bx = barsLeft + i * (BAR_W + BAR_GAP);
        const bool    filled = (i < speedStep);
        tft.fillRect(bx, barsTop, BAR_W, BAR_H,
                     filled ? pal.speedBarFillColor : pal.speedBarEmptyColor);
    }

    // Цифра ступени (0 = выкл, иначе ступень 1..4)
    char buf[3];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(speedStep));

    tft.setFreeFont(Font_Aldrich_28);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(pal.valueColor, pal.screenBg);
    tft.drawString(buf, X, barsTop + BAR_H / 2);

    tft.setTextFont(1);
}

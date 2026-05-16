#include "display/widgets/TimerWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"

void TimerWidget::draw(TFT_eSPI& tft,
                       uint16_t elapsedSec,
                       uint16_t maxSec,
                       const PaletteRGB565& pal) {
    tft.fillRect(X, Y, W, H, pal.screenBg);
    update(tft, elapsedSec, maxSec, pal);
}

void TimerWidget::update(TFT_eSPI& tft,
                          uint16_t elapsedSec,
                          uint16_t maxSec,
                          const PaletteRGB565& pal) {
    tft.fillRect(X, Y, W, H, pal.screenBg);

    // Подпись "сек" (мелко, верхний правый)
    tft.setFreeFont(Font_RobotoMono_12);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.drawString("\xF1\xE5\xEA", X + W, Y); // "сек"

    // Значение "elapsed/max" — два поля разными цветами, правовыровнены
    char bufMax[8];
    snprintf(bufMax, sizeof(bufMax), "/%d", static_cast<int>(maxSec));

    char bufCur[6];
    snprintf(bufCur, sizeof(bufCur), "%d", static_cast<int>(elapsedSec));

    tft.setFreeFont(Font_Aldrich_24);
    tft.setTextDatum(BR_DATUM); // Выравнивание по правому нижнему углу

    // Сначала рисуем "/max" (затухший, правый)
    tft.setTextColor(pal.valueDimColor, pal.screenBg);
    tft.drawString(bufMax, X + W, Y + H);

    // textWidth() использует текущий шрифт (Font_Aldrich_24 уже установлен)
    const int16_t maxTextWidth = tft.textWidth(bufMax);
    tft.setTextColor(pal.valueColor, pal.screenBg);
    tft.setTextDatum(BR_DATUM);
    tft.drawString(bufCur, X + W - maxTextWidth, Y + H);

    tft.setTextFont(1);
}

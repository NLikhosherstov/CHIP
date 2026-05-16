#include "display/widgets/PumpWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"
#include "display/assets/icons.h"

void PumpWidget::draw(TFT_eSPI& tft,
                      uint16_t pulseHz,
                      bool active,
                      const PaletteRGB565& pal) {
    tft.fillRect(X, Y, W, H, pal.screenBg);
    updateFlow(tft, pulseHz, pal);
    drawIcon(tft, active, pal);
}

void PumpWidget::updateFlow(TFT_eSPI& tft,
                             uint16_t pulseHz,
                             const PaletteRGB565& pal) {
    tft.fillRect(X, Y, W, H, pal.screenBg);

    char buf[8];
    formatFlow(buf, sizeof(buf), pulseHz);

    // Подпись "л/ч" (мелко, сверху)
    tft.setFreeFont(Font_RobotoMono_12);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.drawString("\xEB/\xF7", X + 28, Y);  // "л/ч" в Win-1251; при UTF-8 — поменять на u8"л/ч"

    // Значение расхода (крупно, снизу)
    tft.setFreeFont(Font_Aldrich_24);
    tft.setTextColor(pal.valueColor, pal.screenBg);
    tft.drawString(buf, X, Y + 14);

    tft.setTextFont(1);
}

void PumpWidget::drawIcon(TFT_eSPI& tft,
                           bool active,
                           const PaletteRGB565& pal) {
    const uint16_t outerColor = active ? pal.pumpIconActiveColor : pal.pumpIconIdleColor;
    const int16_t ix = ICON_CX - ICON_W / 2;
    const int16_t iy = ICON_CY - ICON_H / 2;

    // Внешний контур иконки насоса
    if (ICON_PUMP_OUTER != nullptr) {
        tft.drawBitmap(ix, iy, ICON_PUMP_OUTER, ICON_PUMP_OUTER_W, ICON_PUMP_OUTER_H,
                       outerColor, pal.screenBg);
    }
    // Внутренняя деталь иконки
    if (ICON_PUMP_INNER != nullptr) {
        tft.drawBitmap(ix + (ICON_W - ICON_PUMP_INNER_W) / 2,
                       iy + (ICON_H - ICON_PUMP_INNER_H) / 2,
                       ICON_PUMP_INNER,
                       ICON_PUMP_INNER_W, ICON_PUMP_INNER_H,
                       outerColor, pal.screenBg);
    }
}

void PumpWidget::formatFlow(char* buf, uint8_t bufSize, uint16_t pulseHz) {
    // Временная заглушка: перевод Гц в условный расход.
    // TODO: заменить на реальный коэффициент калибровки насоса.
    const float lph = static_cast<float>(pulseHz) * 0.054f;
    snprintf(buf, bufSize, "%.2f", lph);
}

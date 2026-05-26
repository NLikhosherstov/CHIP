#include "display/widgets/PumpWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"
#include "display/assets/icons.h"

namespace {
char s_lastFlowBuf[8] = "";

static constexpr int16_t FLOW_VALUE_Y = PumpWidget::Y + 14;
static constexpr int16_t FLOW_VALUE_H = 21;
}  // namespace

void PumpWidget::draw(TFT_eSPI& tft,
                      uint16_t pulseHz,
                      bool active,
                      const PaletteRGB565& pal) {
    s_lastFlowBuf[0] = '\0';
    tft.startWrite();
    tft.fillRect(X, Y, W, H, pal.screenBg);
    tft.endWrite();
    drawIcon(tft, active, pal);

    tft.setFreeFont(Font_RobotoMono_12);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.startWrite();
    tft.drawString("L/H", X + 28, Y);
    tft.endWrite();
    tft.setTextFont(1);

    updateFlow(tft, pulseHz, pal);
}

void PumpWidget::updateFlow(TFT_eSPI& tft,
                             uint16_t pulseHz,
                             const PaletteRGB565& pal) {
    char buf[8];
    formatFlow(buf, sizeof(buf), pulseHz);

    if (!WidgetText::textChanged(buf, s_lastFlowBuf, sizeof(s_lastFlowBuf))) {
        return;
    }

    tft.setFreeFont(Font_Aldrich_24);
    const int16_t textW = tft.textWidth(buf);
    const int16_t clearX = X;
    const int16_t clearW = (textW + 4 > W) ? W : (textW + 4);

    tft.startWrite();
    tft.fillRect(clearX, FLOW_VALUE_Y, clearW, FLOW_VALUE_H, pal.screenBg);
    tft.setTextColor(pal.valueColor, pal.screenBg);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(buf, X, FLOW_VALUE_Y);
    tft.endWrite();

    tft.setTextFont(1);
}

void PumpWidget::drawIcon(TFT_eSPI& tft,
                           bool active,
                           const PaletteRGB565& pal) {
    const uint16_t outerColor = active ? pal.pumpIconActiveColor : pal.pumpIconIdleColor;
    const int16_t ix = ICON_CX - ICON_W / 2;
    const int16_t iy = ICON_CY - ICON_H / 2;

    tft.startWrite();
    if (ICON_PUMP_OUTER != nullptr) {
        tft.drawBitmap(ix, iy, ICON_PUMP_OUTER, ICON_PUMP_OUTER_W, ICON_PUMP_OUTER_H,
                       outerColor, pal.screenBg);
    }
    if (ICON_PUMP_INNER != nullptr) {
        tft.drawBitmap(ix + (ICON_W - ICON_PUMP_INNER_W) / 2,
                       iy + (ICON_H - ICON_PUMP_INNER_H) / 2,
                       ICON_PUMP_INNER,
                       ICON_PUMP_INNER_W, ICON_PUMP_INNER_H,
                       outerColor, pal.screenBg);
    }
    tft.endWrite();
}

void PumpWidget::formatFlow(char* buf, uint8_t bufSize, uint16_t pulseHz) {
    const float lph = static_cast<float>(pulseHz) * 0.054f;
    snprintf(buf, bufSize, "%.2f", lph);
}

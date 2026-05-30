#include "display/widgets/PumpWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"

namespace {
char s_lastFlowBuf[8] = "";

static const SpriteScreenRect kScreen = {PumpWidget::FLOW_PUSH_X, PumpWidget::FLOW_PUSH_Y,
                                         SpriteSlot::Medium};
}  // namespace

void PumpWidget::draw(TFT_eSPI& tft,
                      uint16_t pulseHz,
                      bool active,
                      const PaletteRGB565& pal) {
    (void)active;
    s_lastFlowBuf[0] = '\0';
    tft.startWrite();
    tft.fillRect(X, Y, W, H, pal.screenBg);
    tft.endWrite();

    tft.setFreeFont(Font_small);
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

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::Medium);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kScreen);
        return;
    }

    spr->fillSprite(pal.screenBg);
    spr->setFreeFont(Font_default);
    spr->setTextDatum(TL_DATUM);
    spr->setTextColor(pal.valueColor, pal.screenBg);
    spr->drawString(buf, 0, 0);
    spr->pushSprite(kScreen.x, kScreen.y);
    SpritePool::release(SpriteSlot::Medium);
    tft.setTextFont(1);
}

void PumpWidget::formatFlow(char* buf, uint8_t bufSize, uint16_t pulseHz) {
    const float lph = static_cast<float>(pulseHz) * 0.054f;
    snprintf(buf, bufSize, "%.2f", lph);
}

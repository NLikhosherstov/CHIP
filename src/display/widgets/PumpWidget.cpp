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
                                         SpriteSlot::M};
}  // namespace

void PumpWidget::draw(TFT_eSPI& tft,
                      uint16_t pulseHz,
                      bool active,
                      const PaletteRGB565& pal) {
    (void)active;
    s_lastFlowBuf[1] = '\0';

    tft.loadFont(smooth_font::small);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.startWrite();
    tft.drawString("Л/Ч", X, Y);
    tft.endWrite();
    tft.unloadFont();
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

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::M);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kScreen);
        return;
    }

    spr->fillSprite(pal.screenBg);
    spr->loadFont(smooth_font::def);
    spr->setTextDatum(TL_DATUM);
    spr->setTextColor(pal.valueColor, pal.screenBg);
    spr->fillRect(0, 0, W, H, pal.screenBg);
    spr->drawString(buf, 0, 0);
    spr->pushSprite(kScreen.x, kScreen.y);
    spr->unloadFont();
    SpritePool::release(SpriteSlot::M);
    tft.setTextFont(1);
}

void PumpWidget::formatFlow(char* buf, size_t bufSize, uint16_t pulseHz) {
    const float lph = static_cast<float>(pulseHz) * 0.054f;
    dtostrf(lph, 1, 2, buf);
}

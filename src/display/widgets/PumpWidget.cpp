#include "display/widgets/PumpWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>
#include <string.h>

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
                      uint16_t cycle_period_ms,
                      uint16_t pump_performance,
                      bool active,
                      const PaletteRGB565& pal) {
    (void)active;
    (void)pal;
    s_lastFlowBuf[1] = '\0';

    tft.loadFont(smooth_font::small);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CLR_LABEL, CLR_BG);
    tft.startWrite();
    tft.drawString("Л/Ч", X, Y);
    tft.endWrite();
    tft.unloadFont();
    tft.setTextFont(1);

    updateFlow(tft, cycle_period_ms, pump_performance, pal);
}

void PumpWidget::updateFlow(TFT_eSPI& tft,
                             uint16_t cycle_period_ms,
                             uint16_t pump_performance,
                             const PaletteRGB565& pal) {
    (void)pal;
    char buf[8];
    formatFlow(buf, sizeof(buf), cycle_period_ms, pump_performance);

    if (!WidgetText::textChanged(buf, s_lastFlowBuf, sizeof(s_lastFlowBuf))) {
        return;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::M);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kScreen);
        return;
    }

    spr->fillSprite(CLR_BG);
    spr->loadFont(smooth_font::def);
    spr->setTextDatum(TL_DATUM);
    spr->setTextColor(CLR_VALUE, CLR_BG);
    spr->fillRect(0, 0, W, H, CLR_BG);
    spr->drawString(buf, 0, 0);
    spr->pushSprite(kScreen.x, kScreen.y);
    spr->unloadFont();
    SpritePool::release(SpriteSlot::M);
    tft.setTextFont(1);
}

void PumpWidget::formatFlow(char* buf,
                            size_t bufSize,
                            uint16_t cycle_period_ms,
                            uint16_t pump_performance) {
    if (cycle_period_ms == 0) {
        strncpy(buf, "0.00", bufSize);
        buf[bufSize - 1] = '\0';
        return;
    }

    const float lph =
        3.6f * static_cast<float>(pump_performance) / static_cast<float>(cycle_period_ms);
    dtostrf(lph, 1, 2, buf);
}

#include "display/widgets/CoreWidget.h"

#include <TFT_eSPI.h>
#include <cstdint>
#include <stdio.h>

#include "display/IconDraw.h"
#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"
#include "display/assets/icons.h"

namespace {
char s_lastHexBuf[8]  = "";
char s_lastRoomBuf[6] = "";
char s_lastHumBuf[6]  = "";

void formatHexTempString(char* buf, size_t bufSize, float temp_c) {
    const int t = static_cast<int>(temp_c);
    snprintf(buf, bufSize, "%d", t);
}

static constexpr int16_t kIconSpritePadX =
    (SpritePool::XS_W - icon::medium::BOX_W) / 2;
static constexpr int16_t kIconSpritePadY =
    (SpritePool::XS_H - icon::medium::BOX_H) / 2;

static constexpr int16_t rowValuePushY(int16_t iconY) {
    return iconY + (icon::medium::GLYPH_SIZE - SpritePool::M_H) / 2 - 1;
}

static const SpriteScreenRect kHexScreen = {
    CoreWidget::CX - SpritePool::XL_W / 2,
    CoreWidget::CY - SpritePool::XL_H / 2,
    SpriteSlot::XL};

static const SpriteScreenRect kTempIconScreen = {
    CoreWidget::ROW_ICON_X - kIconSpritePadX,
    CoreWidget::ROOM_ICON_Y - kIconSpritePadY,
    SpriteSlot::XS};

static const SpriteScreenRect kHumIconScreen = {
    CoreWidget::ROW_ICON_X - kIconSpritePadX,
    CoreWidget::HUM_ICON_Y - kIconSpritePadY,
    SpriteSlot::XS};

static const SpriteScreenRect kRoomScreen = {
    CoreWidget::ROW_VALUE_X,
    rowValuePushY(CoreWidget::ROOM_ICON_Y),
    SpriteSlot::M};

static const SpriteScreenRect kHumScreen = {
    CoreWidget::ROW_VALUE_X,
    rowValuePushY(CoreWidget::HUM_ICON_Y),
    SpriteSlot::M};

void pushCoreIcon(TFT_eSPI& tft,
                  const SpriteScreenRect& screen,
                  const char* glyph,
                  uint16_t color,
                  const PaletteRGB565& pal) {
    TFT_eSprite* spr = SpritePool::acquire(screen.slot);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, screen);
        return;
    }

    spr->fillSprite(pal.coreRingBgColor);
    
    int8_t x_shift = -3;
    int8_t y_shift = 0;
    IconDraw::drawIcon(*spr,
                       SpritePool::XS_W / 2 + x_shift,
                       SpritePool::XS_H / 2 + y_shift,
                       icon::medium::font,
                       glyph,
                       color,
                       pal.coreRingBgColor);
    spr->pushSprite(screen.x, screen.y);
    SpritePool::release(screen.slot);
}
}  // namespace

void CoreWidget::resetDrawCache() {
    s_lastHexBuf[0]  = '\0';
    s_lastRoomBuf[0] = '\0';
    s_lastHumBuf[0]  = '\0';
}

void CoreWidget::drawAll(TFT_eSPI& tft,
                         float hexTemp_c,
                         float roomTemp_c,
                         float humidity_pct,
                         const PaletteRGB565& pal) {
    resetDrawCache();
    drawRing(tft, pal);
    drawRoomIcons(tft, pal);
    drawHumIcons(tft, pal);
    drawHexText(tft, hexTemp_c, pal);
    drawRoomText(tft, roomTemp_c, pal);
    drawHumText(tft, humidity_pct, pal);
}

void CoreWidget::updateHexTemp(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    drawHexText(tft, temp_c, pal);
}

void CoreWidget::updateRoomTemp(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    drawRoomText(tft, temp_c, pal);
}

void CoreWidget::updateHumidity(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal) {
    drawHumText(tft, humidity, pal);
}

void CoreWidget::drawRing(TFT_eSPI& tft, const PaletteRGB565& pal) {
    tft.startWrite();
    tft.fillCircle(CX, CY, RING_RADIUS - RING_THICK, pal.coreRingBgColor);
    tft.drawArc(CX, CY, RING_RADIUS, RING_RADIUS - RING_THICK, 0, 360, pal.coreRingColor, pal.coreRingBgColor, true);
    tft.endWrite();
}

void CoreWidget::drawRoomIcons(TFT_eSPI& tft, const PaletteRGB565& pal) {
    pushCoreIcon(tft, kTempIconScreen,
                 icon::medium::TEMP,
                 pal.roomTempColor, pal);
}

void CoreWidget::drawHumIcons(TFT_eSPI& tft, const PaletteRGB565& pal) {
    pushCoreIcon(tft, kHumIconScreen,
                 icon::medium::DROPLET_PCT,
                 pal.humidityColor, pal);
}

void CoreWidget::drawHexText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[8];
    formatHexTempString(buf, sizeof(buf), temp_c);

    if (!WidgetText::textChanged(buf, s_lastHexBuf, sizeof(s_lastHexBuf))) {
        return;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::XL);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kHexScreen);
        return;
    }

    spr->fillSprite(pal.coreRingBgColor);
    spr->loadFont(smooth_font::h1);
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(pal.hexTempColor, pal.coreRingBgColor);
    const int16_t spriteCx = SpritePool::XL_W / 2 - 3;
    const int16_t spriteCy = SpritePool::XL_H / 2 + 5;
    spr->drawString(buf, spriteCx, spriteCy);
    spr->unloadFont();

    spr->pushSprite(kHexScreen.x, kHexScreen.y);
    SpritePool::release(SpriteSlot::XL);
    tft.setTextFont(1);
}

void CoreWidget::drawRoomText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(temp_c));

    if (!WidgetText::textChanged(buf, s_lastRoomBuf, sizeof(s_lastRoomBuf))) {
        return;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::M);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kRoomScreen);
        return;
    }

    spr->fillSprite(pal.coreRingBgColor);
    spr->loadFont(smooth_font::h2);
    spr->setTextDatum(TL_DATUM);
    spr->setTextColor(pal.roomTempColor, pal.coreRingBgColor);
    spr->drawString(buf, 0, 1);
    spr->pushSprite(kRoomScreen.x, kRoomScreen.y);
    spr->unloadFont();
    SpritePool::release(SpriteSlot::M);
    tft.setTextFont(1);
}

void CoreWidget::drawHumText(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(humidity));

    if (!WidgetText::textChanged(buf, s_lastHumBuf, sizeof(s_lastHumBuf))) {
        return;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::M);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kHumScreen);
        return;
    }

    spr->fillSprite(pal.coreRingBgColor);
    spr->loadFont(smooth_font::h2);
    spr->setTextDatum(TL_DATUM);
    spr->setTextColor(pal.humidityColor, pal.coreRingBgColor);
    spr->drawString(buf, 0, 1);
    spr->pushSprite(kHumScreen.x, kHumScreen.y);
    spr->unloadFont();
    SpritePool::release(SpriteSlot::M);
    tft.setTextFont(1);
}

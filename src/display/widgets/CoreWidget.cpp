#include "display/widgets/CoreWidget.h"

#include <TFT_eSPI.h>
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
    return;

    if (t >= 0) {
        snprintf(buf, bufSize, "%03d", t);
    } else {
        snprintf(buf, bufSize, "-%02d", -t);
    }
}

// Единственное место расчёта push-rect спрайтов CoreWidget.
static constexpr int16_t kIconSpritePadX =
    (SpritePool::SMALL_W - icon::CORE_ICON_WIDTH) / 2;
static constexpr int16_t kIconSpritePadY =
    (SpritePool::SMALL_H - icon::CORE_ICON_HEIGHT) / 2;

static constexpr int16_t rowValuePushY(int16_t iconY) {
    // Центрирование Medium 25px относительно иконки 24px; -1 оптическое выравнивание.
    return iconY + (icon::CORE_ICON_HEIGHT - SpritePool::MEDIUM_H) / 2 - 1;
}

static const SpriteScreenRect kHexScreen = {
    CoreWidget::CX - SpritePool::LARGE_W / 2,
    CoreWidget::CY - SpritePool::LARGE_H / 2,
    SpriteSlot::Large};

static const SpriteScreenRect kTempIconScreen = {
    CoreWidget::ROW_ICON_X - kIconSpritePadX,
    CoreWidget::ROOM_ICON_Y - kIconSpritePadY,
    SpriteSlot::Small};

static const SpriteScreenRect kHumIconScreen = {
    CoreWidget::ROW_ICON_X - kIconSpritePadX,
    CoreWidget::HUM_ICON_Y - kIconSpritePadY,
    SpriteSlot::Small};

static const SpriteScreenRect kRoomScreen = {
    CoreWidget::ROW_VALUE_X,
    rowValuePushY(CoreWidget::ROOM_ICON_Y),
    SpriteSlot::Medium};

static const SpriteScreenRect kHumScreen = {
    CoreWidget::ROW_VALUE_X,
    rowValuePushY(CoreWidget::HUM_ICON_Y),
    SpriteSlot::Medium};

void pushCoreIcon(TFT_eSPI& tft,
                  const SpriteScreenRect& screen,
                  const uint8_t* primary,
                  const uint8_t* secondary,
                  uint16_t outerColor,
                  const PaletteRGB565& pal) {
    TFT_eSprite* spr = SpritePool::acquire(screen.slot);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, screen);
        return;
    }

    spr->fillSprite(pal.screenBg);
    IconDraw::drawLayeredBitmap(*spr, kIconSpritePadX, kIconSpritePadY,
                                primary, secondary,
                                icon::CORE_ICON_WIDTH, icon::CORE_ICON_HEIGHT,
                                outerColor, pal.screenBg);
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
    tft.fillCircle(CX, CY, RING_RADIUS - 1, pal.screenBg);
    for (int16_t r = RING_RADIUS; r > RING_RADIUS - RING_THICK; --r) {
        tft.drawCircle(CX, CY, r, pal.coreRingColor);
    }
    tft.endWrite();
}

void CoreWidget::drawRoomIcons(TFT_eSPI& tft, const PaletteRGB565& pal) {
    pushCoreIcon(tft, kTempIconScreen,
                 icon::TEMP_SECONDARY, icon::TEMP_PRIMARY,
                 pal.roomTempColor, pal);
}

void CoreWidget::drawHumIcons(TFT_eSPI& tft, const PaletteRGB565& pal) {
    pushCoreIcon(tft, kHumIconScreen,
                 icon::DROPLET_PCT_SECONDARY, icon::DROPLET_PCT_PRIMARY,
                 pal.humidityColor, pal);
}

void CoreWidget::drawHexText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[8];
    formatHexTempString(buf, sizeof(buf), temp_c);

    if (!WidgetText::textChanged(buf, s_lastHexBuf, sizeof(s_lastHexBuf))) {
        return;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::Large);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kHexScreen);
        return;
    }

    spr->fillSprite(pal.screenBg);
    spr->setFreeFont(Font_h1);
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(pal.hexTempColor, pal.screenBg);
    const int16_t spriteCx = SpritePool::LARGE_W / 2 - 3;
    const int16_t spriteCy = SpritePool::LARGE_H / 2;
    spr->drawString(buf, spriteCx, spriteCy);

    spr->pushSprite(kHexScreen.x, kHexScreen.y);
    SpritePool::release(SpriteSlot::Large);
    tft.setTextFont(1);
}

void CoreWidget::drawRoomText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(temp_c));

    if (!WidgetText::textChanged(buf, s_lastRoomBuf, sizeof(s_lastRoomBuf))) {
        return;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::Medium);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kRoomScreen);
        return;
    }

    spr->fillSprite(pal.screenBg);
    spr->setFreeFont(Font_h2);
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(pal.roomTempColor, pal.screenBg);
    spr->drawString(buf, 0, SpritePool::MEDIUM_H / 2 - 1);
    spr->pushSprite(kRoomScreen.x, kRoomScreen.y);
    SpritePool::release(SpriteSlot::Medium);
    tft.setTextFont(1);
}

void CoreWidget::drawHumText(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(humidity));

    if (!WidgetText::textChanged(buf, s_lastHumBuf, sizeof(s_lastHumBuf))) {
        return;
    }

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::Medium);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kHumScreen);
        return;
    }

    spr->fillSprite(pal.screenBg);
    spr->setFreeFont(Font_h2);
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(pal.humidityColor, pal.screenBg);
    spr->drawString(buf, 0, SpritePool::MEDIUM_H / 2 - 1);
    spr->pushSprite(kHumScreen.x, kHumScreen.y);
    SpritePool::release(SpriteSlot::Medium);
    tft.setTextFont(1);
}

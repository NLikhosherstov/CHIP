#include "display/widgets/CoreWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/WidgetText.h"
#include "display/assets/fonts.h"
#include "display/assets/icons.h"

namespace {
char s_lastHexBuf[8]  = "";
char s_lastRoomBuf[6] = "";
char s_lastHumBuf[6]  = "";

TFT_eSprite* s_hexSprite = nullptr;
bool         s_hexSpriteReady = false;

int16_t maxTextWidth(TFT_eSPI& tft, const char* a, const char* b) {
    const int16_t wa = (a[0] != '\0') ? tft.textWidth(a) : 0;
    const int16_t wb = (b[0] != '\0') ? tft.textWidth(b) : 0;
    return (wa > wb) ? wa : wb;
}
}  // namespace

void CoreWidget::initHexSprite(TFT_eSPI& tft) {
    releaseHexSprite();
    s_hexSprite = new TFT_eSprite(&tft);
    s_hexSpriteReady = s_hexSprite->createSprite(HEX_SPRITE_W, HEX_SPRITE_H);
    if (!s_hexSpriteReady) {
        delete s_hexSprite;
        s_hexSprite = nullptr;
    }
}

void CoreWidget::releaseHexSprite() {
    if (s_hexSprite != nullptr) {
        s_hexSprite->deleteSprite();
        delete s_hexSprite;
        s_hexSprite = nullptr;
    }
    s_hexSpriteReady = false;
}

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
    if (ICON_TEMP_LOW == nullptr) {
        return;
    }
    tft.startWrite();
    tft.drawBitmap(ICON_TEMP_X, ICON_TEMP_Y,
                   ICON_TEMP_LOW,
                   ICON_TEMP_LOW_W, ICON_TEMP_LOW_H,
                   pal.roomTempColor, pal.screenBg);
    tft.endWrite();
}

void CoreWidget::drawHumIcons(TFT_eSPI& tft, const PaletteRGB565& pal) {
    if (ICON_DROPLET_PCT == nullptr) {
        return;
    }
    tft.startWrite();
    tft.drawBitmap(ICON_HUM_X, ICON_HUM_Y,
                   ICON_DROPLET_PCT,
                   ICON_DROPLET_W, ICON_DROPLET_H,
                   pal.humidityColor, pal.screenBg);
    tft.endWrite();
}

void CoreWidget::drawHexText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(temp_c));

    if (!WidgetText::textChanged(buf, s_lastHexBuf, sizeof(s_lastHexBuf))) {
        return;
    }

    if (s_hexSpriteReady && s_hexSprite != nullptr) {
        s_hexSprite->fillSprite(pal.screenBg);
        s_hexSprite->setFreeFont(Font_Aldrich_70);
        s_hexSprite->setTextDatum(MC_DATUM);
        s_hexSprite->setTextColor(pal.hexTempColor, pal.screenBg);
        const int16_t spriteCx = HEX_SPRITE_W / 2;
        const int16_t spriteCy = HEX_SPRITE_H / 2;
        s_hexSprite->drawString(buf, spriteCx, spriteCy);

        s_hexSprite->setFreeFont(Font_RobotoMono_40);
        s_hexSprite->setTextDatum(TL_DATUM);
        const int16_t degX = spriteCx + s_hexSprite->textWidth(buf) / 2 + 2;
        s_hexSprite->drawString("\xB0", degX, 2);

        const int16_t pushX = CX - HEX_SPRITE_W / 2;
        const int16_t pushY = CY - HEX_SPRITE_H / 2;
        s_hexSprite->pushSprite(pushX, pushY);
        tft.setTextFont(1);
        return;
    }

    tft.setFreeFont(Font_Aldrich_70);
    const int16_t textW = maxTextWidth(tft, buf, "");
    const int16_t clearW = textW + 28;
    const int16_t clearH = HEX_CLEAR_H;
    const int16_t clearX = CX - clearW / 2;
    const int16_t clearY = HEX_CLEAR_Y;

    tft.startWrite();
    tft.fillRect(clearX, clearY, clearW, clearH, pal.screenBg);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(pal.hexTempColor, pal.screenBg);
    tft.drawString(buf, CX, CY);

    const int16_t degX = CX + tft.textWidth(buf) / 2 + 2;
    tft.setFreeFont(Font_RobotoMono_40);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("\xB0", degX, HEX_CLEAR_Y + 4);
    tft.endWrite();

    tft.setTextFont(1);
}

void CoreWidget::drawRoomText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(temp_c));

    if (!WidgetText::textChanged(buf, s_lastRoomBuf, sizeof(s_lastRoomBuf))) {
        return;
    }

    tft.setFreeFont(Font_Aldrich_34);
    const int16_t textW = maxTextWidth(tft, buf, "");
    const int16_t clearX = CX + 8 - textW / 2 - 2;
    const int16_t clearW = textW + 8;
    const int16_t clearY = ROOM_CLEAR_Y;
    const int16_t clearH = ROOM_CLEAR_H;

    tft.startWrite();
    tft.fillRect(clearX, clearY, clearW, clearH, pal.screenBg);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(pal.roomTempColor, pal.screenBg);
    tft.drawString(buf, CX + 8, ROOM_CLEAR_Y + ROOM_CLEAR_H / 2);
    tft.endWrite();

    tft.setTextFont(1);
}

void CoreWidget::drawHumText(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(humidity));

    if (!WidgetText::textChanged(buf, s_lastHumBuf, sizeof(s_lastHumBuf))) {
        return;
    }

    tft.setFreeFont(Font_Aldrich_34);
    const int16_t textW = maxTextWidth(tft, buf, "");
    const int16_t clearX = CX + 8 - textW / 2 - 2;
    const int16_t clearW = textW + 8;
    const int16_t clearY = HUM_CLEAR_Y;
    const int16_t clearH = HUM_CLEAR_H;

    tft.startWrite();
    tft.fillRect(clearX, clearY, clearW, clearH, pal.screenBg);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(pal.humidityColor, pal.screenBg);
    tft.drawString(buf, CX + 8, HUM_CLEAR_Y + HUM_CLEAR_H / 2);
    tft.endWrite();

    tft.setTextFont(1);
}

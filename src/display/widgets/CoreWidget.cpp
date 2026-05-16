#include "display/widgets/CoreWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"
#include "display/assets/icons.h"

void CoreWidget::drawAll(TFT_eSPI& tft,
                         float hexTemp_c,
                         float roomTemp_c,
                         float humidity_pct,
                         const PaletteRGB565& pal) {
    drawRing(tft, pal);
    drawHexText(tft, hexTemp_c,   pal);
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

// ─── Приватные хелперы ────────────────────────────────────────────────────────

void CoreWidget::drawRing(TFT_eSPI& tft, const PaletteRGB565& pal) {
    // Заливаем внутреннюю область кольца фоном экрана
    tft.fillCircle(CX, CY, RING_RADIUS - 1, pal.screenBg);
    // Рисуем три концентрических окружности для кольца толщиной RING_THICK
    for (int16_t r = RING_RADIUS; r > RING_RADIUS - RING_THICK; --r) {
        tft.drawCircle(CX, CY, r, pal.coreRingColor);
    }
}

void CoreWidget::drawHexText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(temp_c));

    // Очищаем область перед перерисовкой (предотвращает артефакты при смене ширины текста)
    tft.fillRect(HEX_CLEAR_X, HEX_CLEAR_Y, HEX_CLEAR_W, HEX_CLEAR_H, pal.screenBg);

    tft.setFreeFont(Font_Aldrich_70);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(pal.hexTempColor, pal.screenBg);
    tft.drawString(buf, CX, CY);

    // Символ "°" в шрифте Roboto Mono (небольшое смещение от числа)
    // textWidth() использует текущий шрифт (Font_Aldrich_70 уже установлен)
    const int16_t degX = CX + tft.textWidth(buf) / 2 + 2;
    tft.setFreeFont(Font_RobotoMono_40);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("\xB0", degX, HEX_CLEAR_Y + 4);

    tft.setTextFont(1); // Сброс к встроенному шрифту
}

void CoreWidget::drawRoomText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(temp_c));

    tft.fillRect(ROOM_CLEAR_X, ROOM_CLEAR_Y, ROOM_CLEAR_W, ROOM_CLEAR_H, pal.screenBg);

    // Иконка термометра (температура воздуха)
    if (ICON_TEMP_LOW != nullptr) {
        tft.drawBitmap(ICON_TEMP_X, ICON_TEMP_Y,
                       ICON_TEMP_LOW,
                       ICON_TEMP_LOW_W, ICON_TEMP_LOW_H,
                       pal.roomTempColor, pal.screenBg);
    }

    tft.setFreeFont(Font_Aldrich_34);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(pal.roomTempColor, pal.screenBg);
    tft.drawString(buf, CX + 8, ROOM_CLEAR_Y + ROOM_CLEAR_H / 2);

    tft.setTextFont(1);
}

void CoreWidget::drawHumText(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d", static_cast<int>(humidity));

    tft.fillRect(HUM_CLEAR_X, HUM_CLEAR_Y, HUM_CLEAR_W, HUM_CLEAR_H, pal.screenBg);

    // Иконка капли с процентом (влажность)
    if (ICON_DROPLET_PCT != nullptr) {
        tft.drawBitmap(ICON_HUM_X, ICON_HUM_Y,
                       ICON_DROPLET_PCT,
                       ICON_DROPLET_W, ICON_DROPLET_H,
                       pal.humidityColor, pal.screenBg);
    }

    tft.setFreeFont(Font_Aldrich_34);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(pal.humidityColor, pal.screenBg);
    tft.drawString(buf, CX + 8, HUM_CLEAR_Y + HUM_CLEAR_H / 2);

    tft.setTextFont(1);
}

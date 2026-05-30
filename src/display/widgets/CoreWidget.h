#pragma once

#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Центральный виджет: кольцо + три строки температурных данных.
struct CoreWidget {
    // Центральный элемент
    static constexpr int16_t CX          = 159;
    static constexpr int16_t CY          = 119;
    static constexpr int16_t RING_RADIUS = 107;
    static constexpr int16_t RING_THICK  = 4;

    // Строки room / humidity: top-left иконки 24×24 и left edge спрайта значения (Medium 85×25)
    static constexpr int16_t ROW_ICON_X  = 110;
    static constexpr int16_t ROW_VALUE_X = 139;
    
    static constexpr int16_t ROOM_ICON_Y = 50;
    static constexpr int16_t HUM_ICON_Y  = 166;

    static void drawAll(TFT_eSPI& tft,
                        float hexTemp_c,
                        float roomTemp_c,
                        float humidity_pct,
                        const PaletteRGB565& pal);

    static void resetDrawCache();

    static void updateHexTemp(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal);
    static void updateRoomTemp(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal);
    static void updateHumidity(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal);

private:
    static void drawRing(TFT_eSPI& tft, const PaletteRGB565& pal);
    static void drawRoomIcons(TFT_eSPI& tft, const PaletteRGB565& pal);
    static void drawHumIcons(TFT_eSPI& tft, const PaletteRGB565& pal);
    static void drawHexText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal);
    static void drawRoomText(TFT_eSPI& tft, float temp_c, const PaletteRGB565& pal);
    static void drawHumText(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal);
};

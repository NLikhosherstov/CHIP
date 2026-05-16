#pragma once
#include <stdint.h>

class TFT_eSPI;
struct PaletteRGB565;

// Центральный виджет: кольцо + три строки температурных данных.
// Все методы статические — экземпляр объекта не нужен, RAM не расходуется.
//
// Расположение (из макета Figma, 320x240):
//   Центр кольца:      x=159, y=119
//   Внешний радиус:    107px (кольцо 214px)
//   Толщина кольца:    3px (три концентрических окружности)
//   Область контента:  170x170, начиная с (74, 34)
struct CoreWidget {
    // Геометрия кольца
    static constexpr int16_t CX          = 159;
    static constexpr int16_t CY          = 119;
    static constexpr int16_t RING_RADIUS = 107;
    static constexpr int16_t RING_THICK  = 3;

    // Области очистки для каждого текстового поля (fillRect перед перерисовкой)
    static constexpr int16_t ROOM_CLEAR_X  = 120, ROOM_CLEAR_Y  = 38;
    static constexpr int16_t ROOM_CLEAR_W  = 78,  ROOM_CLEAR_H  = 40;

    static constexpr int16_t HEX_CLEAR_X   = 88,  HEX_CLEAR_Y   = 82;
    static constexpr int16_t HEX_CLEAR_W   = 142, HEX_CLEAR_H   = 75;

    static constexpr int16_t HUM_CLEAR_X   = 122, HUM_CLEAR_Y   = 158;
    static constexpr int16_t HUM_CLEAR_W   = 74,  HUM_CLEAR_H   = 40;

    // Позиции иконок (левый верхний угол bitmap)
    static constexpr int16_t ICON_TEMP_X   = 109, ICON_TEMP_Y   = 42;  // 24x24
    static constexpr int16_t ICON_HUM_X    = 110, ICON_HUM_Y    = 163; // 18x25

    // Полная отрисовка кольца и содержимого (вызывается при смене режима/первом входе)
    static void drawAll(TFT_eSPI& tft,
                        float hexTemp_c,
                        float roomTemp_c,
                        float humidity_pct,
                        const PaletteRGB565& pal);

    // Частичные обновления отдельных полей (вызываются при изменении значения)
    static void updateHexTemp (TFT_eSPI& tft, float temp_c,    const PaletteRGB565& pal);
    static void updateRoomTemp(TFT_eSPI& tft, float temp_c,    const PaletteRGB565& pal);
    static void updateHumidity(TFT_eSPI& tft, float humidity,  const PaletteRGB565& pal);

private:
    static void drawRing   (TFT_eSPI& tft, const PaletteRGB565& pal);
    static void drawHexText(TFT_eSPI& tft, float temp_c,   const PaletteRGB565& pal);
    static void drawRoomText(TFT_eSPI& tft, float temp_c,  const PaletteRGB565& pal);
    static void drawHumText(TFT_eSPI& tft, float humidity, const PaletteRGB565& pal);
};

#pragma once
#include <stdint.h>
#include "system/SystemState.h"

// Конвертация RGB888 → RGB565
#define RGB565(r, g, b) \
    static_cast<uint16_t>( \
        ((static_cast<uint16_t>(r) & 0xF8u) << 8u) | \
        ((static_cast<uint16_t>(g) & 0xFCu) << 3u) | \
        ((static_cast<uint16_t>(b) & 0xF8u) >> 3u))
        
// Базовые цвета
static constexpr uint16_t CLR_BG           = RGB565(0x00, 0x00, 0x07); // #000308
static constexpr uint16_t CLR_HEX_TEMP     = RGB565(0xDF, 0xDF, 0xDF); // #dfdfdf
static constexpr uint16_t CLR_ROOM         = RGB565(0xA4, 0xA6, 0xAA); // #a4a6aa
static constexpr uint16_t CLR_LABEL        = RGB565(0x80, 0x80, 0x80); // #808080
static constexpr uint16_t CLR_VALUE        = RGB565(0xDF, 0xDF, 0xDF); // #dfdfdf
static constexpr uint16_t CLR_VALUE_DIM    = RGB565(0x55, 0x55, 0x55); // #555555
static constexpr uint16_t CLR_ICON_IDLE    = RGB565(0x80, 0x80, 0x80);
static constexpr uint16_t CLR_ACCENT_GREEN = RGB565(0x26, 0xFF, 0x80); // #26FF00
static constexpr uint16_t CLR_ACCENT_BLUE  = RGB565(0x00, 0xE5, 0xFF); // #00E5FF
static constexpr uint16_t CLR_SELECTION    = RGB565(0x00, 0x04, 0x08); // #000408

// Специальные цвета
static constexpr uint16_t CLR_IDLE_RING      = RGB565(0xAE, 0xB9, 0xD0);
static constexpr uint16_t CLR_IDLE_RING_BG   = RGB565(0x00, 0x00, 0x07); //RGB565(0x08, 0x04, 0x08);
static constexpr uint16_t CLR_AUTO_RING      = RGB565(0x00, 0xBC, 0xD1);
static constexpr uint16_t CLR_AUTO_RING_BG   = RGB565(0x00, 0x04, 0x08);
static constexpr uint16_t CLR_MANUAL_RING    = RGB565(0xAE, 0xB9, 0xD0); //RGB565(0xFF, 0x0B, 0x17); // #FF0B17
static constexpr uint16_t CLR_MANUAL_RING_BG = RGB565(0x08, 0x04, 0x08); //RGB565(0x10, 0x03, 0x03); // #100303
static constexpr uint16_t CLR_STOP_RING      = RGB565(0xFF, 0xB0, 0x00);
static constexpr uint16_t CLR_STOP_RING_BG   = RGB565(0x08, 0x04, 0x08); // #080408
static constexpr uint16_t CLR_SPEED_BAR_BG   = RGB565(0x70, 0x70, 0x70); // #6060B0
static constexpr uint16_t CLR_SPEED_BAR_FG   = RGB565(0xB0, 0xB0, 0xB0);
static constexpr uint16_t CLR_PUMP_ICON_BG   = RGB565(0x60, 0x60, 0xB0); // #2E6FD1
static constexpr uint16_t CLR_IGN_ICON_BG    = RGB565(0xEC, 0x3C, 0x13); // #EC6C13
static constexpr uint16_t CLR_ICON_INACTIVE  = RGB565(0x10, 0x10, 0x10); // #303030

// Полная палитра цветов одного режима работы.
struct PaletteRGB565 {
    uint16_t screenBg;            // Фон экрана
    uint16_t coreRingColor;       // Контур core-кольца 
    uint16_t coreRingBgColor;     // Фон core-кольца
    uint16_t hexTempColor;        // Температура теплообменника
    uint16_t roomTempColor;       // Температура помещения
    uint16_t humidityColor;       // Влажность
    uint16_t modeNameColor;       // Режим ("IDLE", "AUTO", "РУЧН", "СТОП")
    uint16_t labelColor;          // Подписи ("режим", "л/ч", "сек", "скорость")
    uint16_t valueColor;          // Primary-значения вспомогательных виджетов
    uint16_t valueDimColor;       // Secondary-значения вспомогательных виджетов
    uint16_t speedBarFillColor;   // Заполнение сегмента шкалы скорости вентилятора
    uint16_t speedBarBorderColor; // Граница сегмента шкалы скорости вентилятора
    uint16_t pumpIconActiveColor; // Иконка насоса — насос работает
    uint16_t pumpIconIdleColor;   // Иконка насоса — насос выключен
    uint16_t ignitionActiveColor; // Иконка свечи зажигания — ON
    uint16_t ignitionIdleColor;   // Иконка свечи зажигания — OFF
};

class PaletteManager {
public:
    // Возвращает const-ссылку на палитру текущего режима.
    static const PaletteRGB565& get(SystemState::AutomationState state);

private:
    // Массив k_palettes размещается компилятором во Flash (.rodata).
    static const PaletteRGB565 k_palettes[5];
};

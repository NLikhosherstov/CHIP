#pragma once
#include <stdint.h>
#include "system/SystemState.h"

// Конвертация RGB888 → RGB565
#define RGB565(r, g, b) \
    static_cast<uint16_t>( \
        ((static_cast<uint16_t>(r) & 0xF8u) << 8u) | \
        ((static_cast<uint16_t>(g) & 0xFCu) << 3u) | \
        ((static_cast<uint16_t>(b) & 0xF8u) >> 3u))

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
    uint16_t speedBarFillColor;   // Заполненный сегмент шкалы скорости вентилятора
    uint16_t speedBarEmptyColor;  // Пустой сегмент шкалы скорости вентилятора
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

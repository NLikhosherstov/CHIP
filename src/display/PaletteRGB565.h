#pragma once
#include <stdint.h>
#include <stddef.h>
#include "system/SystemState.h"

namespace PaletteColor {

constexpr uint32_t hexCharValue(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint32_t>(c - '0');
    if (c >= 'A' && c <= 'F') return static_cast<uint32_t>(c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return static_cast<uint32_t>(c - 'a' + 10);
    return 0u;
}

constexpr uint32_t parseHexRgb(const char* s, size_t len) {
    size_t i = (len > 0u && s[0] == '#') ? 1u : 0u;
    uint32_t value = 0u;
    for (; i < len; ++i) {
        value = (value << 4u) | hexCharValue(s[i]);
    }
    return value;
}

constexpr uint16_t fromRgb888(uint32_t rgb) {
    const uint8_t r = static_cast<uint8_t>((rgb >> 16) & 0xFFu);
    const uint8_t g = static_cast<uint8_t>((rgb >> 8) & 0xFFu);
    const uint8_t b = static_cast<uint8_t>(rgb & 0xFFu);
    return static_cast<uint16_t>(
        ((static_cast<uint16_t>(r) & 0xF8u) << 8u) |
        ((static_cast<uint16_t>(g) & 0xFCu) << 3u) |
        ((static_cast<uint16_t>(b) & 0xF8u) >> 3u));
}

}  // namespace PaletteColor

// Конвертация RGB888 → RGB565 на этапе компиляции.
// RGB565("#RRGGBB") или RGB565(0xRRGGBB)
constexpr uint16_t RGB565(uint32_t rgb888) {
    return PaletteColor::fromRgb888(rgb888);
}

template <size_t N>
constexpr uint16_t RGB565(const char (&hex)[N]) {
    return PaletteColor::fromRgb888(PaletteColor::parseHexRgb(hex, N - 1u));
}

// Базовые цвета
static constexpr uint16_t CLR_BG             = RGB565("#000007"); // #000007
static constexpr uint16_t CLR_VALUE          = RGB565("#DFDFDF"); // #DFDFDF
static constexpr uint16_t CLR_VALUE_DIM      = RGB565("#555555"); // #555555
static constexpr uint16_t CLR_LABEL          = RGB565("#808080"); // #808080
static constexpr uint16_t CLR_ACCENT_GREEN   = RGB565("#26FF80"); // #26FF80
static constexpr uint16_t CLR_ACCENT_BLUE    = RGB565("#00E5FF"); // #00E5FF
static constexpr uint16_t CLR_ICON_INACTIVE  = RGB565("#101010"); // #101010
static constexpr uint16_t CLR_SELECTION      = RGB565("#000408"); // #000408

// Специфические цвета
static constexpr uint16_t CLR_HEX_TEMP       = CLR_VALUE;
static constexpr uint16_t CLR_ROOM           = RGB565("#A4A6AA"); // #A4A6AA
static constexpr uint16_t CLR_PUMP_ICON      = RGB565("#6060B0"); // #6060B0
static constexpr uint16_t CLR_IGN_ICON       = RGB565("#EC3C13"); // #EC3C13
static constexpr uint16_t CLR_SPEED_BAR_BG   = RGB565("#707070"); // #707070
static constexpr uint16_t CLR_SPEED_BAR_FG   = RGB565("#B0B0B0"); // #B0B0B0

// Динамические (зависят от режима) цвета
static constexpr uint16_t CLR_IDLE_RING      = RGB565("#AEB9D0"); // #AEB9D0
static constexpr uint16_t CLR_IDLE_RING_BG   = RGB565("#000007"); // #000007
static constexpr uint16_t CLR_AUTO_RING      = RGB565("#00BCD1"); // #00BCD1
static constexpr uint16_t CLR_AUTO_RING_BG   = RGB565("#000408"); // #000408
static constexpr uint16_t CLR_MANUAL_RING    = RGB565("#AEB9D0"); // #AEB9D0
static constexpr uint16_t CLR_MANUAL_RING_BG = RGB565("#080408"); // #080408
static constexpr uint16_t CLR_STOP_RING      = RGB565("#FFB000"); // #FFB000
static constexpr uint16_t CLR_STOP_RING_BG   = RGB565("#080408"); // #080408

// Акцентные цвета режима: кольцо CoreWidget и название режима.
struct PaletteRGB565 {
    uint16_t accentFG;  // Контур core-кольца, название режима
    uint16_t accentBG;  // Фон core-кольца и спрайтов внутри него
};

class PaletteManager {
public:
    // Возвращает const-ссылку на палитру текущего режима.
    static const PaletteRGB565& get(SystemState::AutomationState state);

private:
    // Массив k_palettes размещается компилятором во Flash (.rodata).
    static const PaletteRGB565 k_palettes[5];
};

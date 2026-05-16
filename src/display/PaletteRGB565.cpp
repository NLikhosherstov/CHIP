#include "display/PaletteRGB565.h"

// Базовые цвета
static constexpr uint16_t CLR_BG           = RGB565(0x00, 0x0B, 0x16); // #000b16
static constexpr uint16_t CLR_HEX_TEMP     = RGB565(0xDF, 0xDF, 0xDF); // #dfdfdf
static constexpr uint16_t CLR_ROOM         = RGB565(0xA4, 0xA6, 0xAA); // #a4a6aa
static constexpr uint16_t CLR_LABEL        = RGB565(0x80, 0x80, 0x80); // #808080
static constexpr uint16_t CLR_VALUE        = RGB565(0xDF, 0xDF, 0xDF);
static constexpr uint16_t CLR_VALUE_DIM    = RGB565(0x55, 0x55, 0x55); // #555555
static constexpr uint16_t CLR_ICON_IDLE    = RGB565(0x80, 0x80, 0x80);

// Акцентные цвета
static constexpr uint16_t CLR_IDLE_RING      = RGB565(0xAE, 0xB9, 0xD0);
static constexpr uint16_t CLR_IDLE_RING_BG   = RGB565(0x1A, 0x20, 0x2D);
static constexpr uint16_t CLR_AUTO_RING      = RGB565(0x00, 0xBC, 0xD1);
static constexpr uint16_t CLR_AUTO_RING_BG   = RGB565(0x00, 0x21, 0x2D);
static constexpr uint16_t CLR_MANUAL_RING    = RGB565(0xA3, 0x55, 0x2E);
static constexpr uint16_t CLR_MANUAL_RING_BG = RGB565(0x24, 0x18, 0x12);
static constexpr uint16_t CLR_STOP_RING      = RGB565(0xFF, 0xB0, 0x00);
static constexpr uint16_t CLR_STOP_RING_BG   = RGB565(0x19, 0x1C, 0x14);

// ─── Таблица палитр: индексируется значением AutomationState (0..4) ──────────
const PaletteRGB565 PaletteManager::k_palettes[5] = {
    // [0] STATE_IDLE
    {
        CLR_BG,          CLR_IDLE_RING,   CLR_IDLE_RING_BG,
        CLR_HEX_TEMP,    CLR_ROOM,        CLR_ROOM,
        CLR_VALUE,       CLR_LABEL,       CLR_VALUE,       CLR_VALUE_DIM,
        CLR_ICON_IDLE,   CLR_ICON_IDLE,   // иконка насоса (неактивен)
        CLR_IDLE_RING,   CLR_VALUE_DIM,   // шкала скорости
        CLR_ICON_IDLE,   CLR_ICON_IDLE,   // иконка свечи (неактивна)
    },
    // [1] STATE_AUTO_START
    {
        CLR_BG,          CLR_AUTO_RING,   CLR_AUTO_RING_BG,
        CLR_HEX_TEMP,    CLR_ROOM,        CLR_ROOM,
        CLR_VALUE,       CLR_LABEL,       CLR_VALUE,       CLR_VALUE_DIM,
        CLR_AUTO_RING,   CLR_ICON_IDLE,   // насос активен — цвет акцента
        CLR_AUTO_RING,   CLR_VALUE_DIM,
        CLR_AUTO_RING,   CLR_ICON_IDLE,   // свеча активна в AUTO_START
    },
    // [2] STATE_AUTO_TSTAT (идентично AUTO_START; для разделения — правь здесь)
    {
        CLR_BG,          CLR_AUTO_RING,   CLR_AUTO_RING_BG,
        CLR_HEX_TEMP,    CLR_ROOM,        CLR_ROOM,
        CLR_VALUE,       CLR_LABEL,       CLR_VALUE,       CLR_VALUE_DIM,
        CLR_AUTO_RING,   CLR_ICON_IDLE,
        CLR_AUTO_RING,   CLR_VALUE_DIM,
        CLR_ICON_IDLE,   CLR_ICON_IDLE,   // свеча уже не горит в режиме ТСТАТ
    },
    // [3] STATE_MANUAL
    {
        CLR_BG,          CLR_MANUAL_RING, CLR_MANUAL_RING_BG,
        CLR_HEX_TEMP,    CLR_ROOM,        CLR_ROOM,
        CLR_VALUE,       CLR_LABEL,       CLR_VALUE,       CLR_VALUE_DIM,
        CLR_MANUAL_RING, CLR_ICON_IDLE,
        CLR_MANUAL_RING, CLR_VALUE_DIM,
        CLR_ICON_IDLE,   CLR_ICON_IDLE,
    },
    // [4] STATE_STOP
    {
        CLR_BG,          CLR_STOP_RING,   CLR_STOP_RING_BG,
        CLR_HEX_TEMP,    CLR_ROOM,        CLR_ROOM,
        CLR_VALUE,       CLR_LABEL,       CLR_VALUE,       CLR_VALUE_DIM,
        CLR_ICON_IDLE,   CLR_ICON_IDLE,
        CLR_STOP_RING,   CLR_VALUE_DIM,
        CLR_ICON_IDLE,   CLR_ICON_IDLE,
    },
};

const PaletteRGB565& PaletteManager::get(SystemState::AutomationState state) {
    const auto idx = static_cast<uint8_t>(state);
    return (idx < 5u) ? k_palettes[idx] : k_palettes[0];
}

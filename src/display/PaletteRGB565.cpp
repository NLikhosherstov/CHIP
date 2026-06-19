#include "display/PaletteRGB565.h"


// ─── Таблица палитр: индексируется значением AutomationState (0..4) ──────────
const PaletteRGB565 PaletteManager::k_palettes[5] = {
    // [0] STATE_IDLE
    { CLR_IDLE_RING,   CLR_IDLE_RING_BG },
    // [1] STATE_AUTO_START
    { CLR_AUTO_RING,   CLR_AUTO_RING_BG },
    // [2] STATE_AUTO_TSTAT
    { CLR_AUTO_RING,   CLR_AUTO_RING_BG },
    // [3] STATE_MANUAL
    { CLR_MANUAL_RING, CLR_MANUAL_RING_BG },
    // [4] STATE_STOP
    { CLR_STOP_RING,   CLR_STOP_RING_BG },
};

const PaletteRGB565& PaletteManager::get(SystemState::AutomationState state) {
    const auto idx = static_cast<uint8_t>(state);
    return (idx < 5u) ? k_palettes[idx] : k_palettes[0];
}

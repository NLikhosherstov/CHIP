#include "display/PaletteRGB565.h"


// ─── Таблица палитр: индексируется значением AutomationState (0..4) ──────────
const PaletteRGB565 PaletteManager::k_palettes[5] = {
    // [0] STATE_IDLE
    {
        /*screenBg*/            CLR_BG,
        /*coreRingColor*/       CLR_IDLE_RING,
        /*coreRingBgColor*/     CLR_IDLE_RING_BG,
        /*hexTempColor*/        CLR_HEX_TEMP,
        /*roomTempColor*/       CLR_ROOM,
        /*humidityColor*/       CLR_ROOM,
        /*modeNameColor*/       CLR_VALUE,
        /*labelColor*/          CLR_LABEL,
        /*valueColor*/          CLR_VALUE,
        /*valueDimColor*/       CLR_VALUE_DIM,
        /*speedBarFillColor*/   CLR_SPEED_BAR_BG,
        /*speedBarBorderColor*/ CLR_SPEED_BAR_FG,
        /*pumpIconActiveColor*/ CLR_PUMP_ICON_BG,
        /*pumpIconIdleColor*/   CLR_ICON_INACTIVE,
        /*ignitionActiveColor*/ CLR_IGN_ICON_BG,
        /*ignitionIdleColor*/   CLR_ICON_INACTIVE,
    },
    // [1] STATE_AUTO_START
    {
        /*screenBg*/            CLR_BG,
        /*coreRingColor*/       CLR_AUTO_RING,
        /*coreRingBgColor*/     CLR_AUTO_RING_BG,
        /*hexTempColor*/        CLR_HEX_TEMP,
        /*roomTempColor*/       CLR_ROOM,
        /*humidityColor*/       CLR_ROOM,
        /*modeNameColor*/       CLR_AUTO_RING,
        /*labelColor*/          CLR_LABEL,
        /*valueColor*/          CLR_VALUE,
        /*valueDimColor*/       CLR_VALUE_DIM,
        /*speedBarFillColor*/   CLR_SPEED_BAR_BG,
        /*speedBarBorderColor*/ CLR_SPEED_BAR_FG,
        /*pumpIconActiveColor*/ CLR_PUMP_ICON_BG,
        /*pumpIconIdleColor*/   CLR_ICON_INACTIVE,
        /*ignitionActiveColor*/ CLR_IGN_ICON_BG,
        /*ignitionIdleColor*/   CLR_ICON_INACTIVE,
    },
    // [2] STATE_AUTO_TSTAT
    {
        /*screenBg*/            CLR_BG,
        /*coreRingColor*/       CLR_AUTO_RING,
        /*coreRingBgColor*/     CLR_AUTO_RING_BG,
        /*hexTempColor*/        CLR_HEX_TEMP,
        /*roomTempColor*/       CLR_ROOM,
        /*humidityColor*/       CLR_ROOM,
        /*modeNameColor*/       CLR_AUTO_RING,
        /*labelColor*/          CLR_LABEL,
        /*valueColor*/          CLR_VALUE,
        /*valueDimColor*/       CLR_VALUE_DIM,
        /*speedBarFillColor*/   CLR_SPEED_BAR_BG,
        /*speedBarBorderColor*/ CLR_SPEED_BAR_FG,
        /*pumpIconActiveColor*/ CLR_PUMP_ICON_BG,
        /*pumpIconIdleColor*/   CLR_ICON_INACTIVE,
        /*ignitionActiveColor*/ CLR_IGN_ICON_BG,
        /*ignitionIdleColor*/   CLR_ICON_INACTIVE,
    },
    // [3] STATE_MANUAL
    {
        /*screenBg*/            CLR_BG,
        /*coreRingColor*/       CLR_MANUAL_RING,
        /*coreRingBgColor*/     CLR_MANUAL_RING_BG,
        /*hexTempColor*/        CLR_HEX_TEMP,
        /*roomTempColor*/       CLR_ROOM,
        /*humidityColor*/       CLR_ROOM,
        /*modeNameColor*/       CLR_MANUAL_RING,
        /*labelColor*/          CLR_LABEL,
        /*valueColor*/          CLR_VALUE,
        /*valueDimColor*/       CLR_VALUE_DIM,
        /*speedBarFillColor*/   CLR_SPEED_BAR_BG,
        /*speedBarBorderColor*/ CLR_SPEED_BAR_FG,
        /*pumpIconActiveColor*/ CLR_PUMP_ICON_BG,
        /*pumpIconIdleColor*/   CLR_ICON_INACTIVE,
        /*ignitionActiveColor*/ CLR_IGN_ICON_BG,
        /*ignitionIdleColor*/   CLR_ICON_INACTIVE,
    },
    // [4] STATE_STOP
    {
        /*screenBg*/            CLR_BG,
        /*coreRingColor*/       CLR_STOP_RING,
        /*coreRingBgColor*/     CLR_STOP_RING_BG,
        /*hexTempColor*/        CLR_HEX_TEMP,
        /*roomTempColor*/       CLR_ROOM,
        /*humidityColor*/       CLR_ROOM,
        /*modeNameColor*/       CLR_STOP_RING,
        /*labelColor*/          CLR_LABEL,
        /*valueColor*/          CLR_VALUE,
        /*valueDimColor*/       CLR_VALUE_DIM,
        /*speedBarFillColor*/   CLR_SPEED_BAR_BG,
        /*speedBarBorderColor*/ CLR_SPEED_BAR_FG,
        /*pumpIconActiveColor*/ CLR_PUMP_ICON_BG,
        /*pumpIconIdleColor*/   CLR_ICON_INACTIVE,
        /*ignitionActiveColor*/ CLR_IGN_ICON_BG,
        /*ignitionIdleColor*/   CLR_ICON_INACTIVE,
    },
};

const PaletteRGB565& PaletteManager::get(SystemState::AutomationState state) {
    const auto idx = static_cast<uint8_t>(state);
    return (idx < 5u) ? k_palettes[idx] : k_palettes[0];
}

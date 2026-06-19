#include "display/screens/HomeScreen.h"

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/widgets/CoreWidget.h"
#include "display/widgets/ModeWidget.h"
#include "display/widgets/PumpIndicator.h"
#include "display/widgets/PumpWidget.h"
#include "display/widgets/QuickMenuWidget.h"
#include "display/widgets/SpeedWidget.h"
#include "display/widgets/TimerWidget.h"
#include "display/widgets/IgnitionIndicator.h"
#include "system/SystemState.h"
#include "system/ConfigManager.h"

HomeScreen::HomeScreen()
    : m_lastState(SystemState::AutomationState::STATE_IDLE)
    , m_lastHexDisplay(-999)
    , m_lastRoomDisplay(-999)
    , m_lastHumDisplay(-999)
    , m_lastSpeedIndex(0xFF)
    , m_lastTimerSec(0xFFFFu)
    , m_lastCyclePeriodMs(0xFFFFu)
    , m_lastIgnActive(false)
    , m_lastPumpActive(false)
    , m_fullRedrawNeeded(true)
    , m_qmVisible(false)
    , m_qmSelectedItem(0)
    , m_qmLastActivityMs(0)
{}

void HomeScreen::onEnter(TFT_eSPI& tft,
                          const SystemState& state,
                          const ConfigManager& cfg) {
    m_fullRedrawNeeded = true;
    m_qmVisible = false;
    SpritePool::init(tft);
    drawAll(tft, state, cfg);
}

void HomeScreen::onExit() {
    m_qmVisible = false;
    m_fullRedrawNeeded = true;
    SpritePool::shutdown();
}

void HomeScreen::tick(TFT_eSPI& tft,
                      const SystemState& state,
                      const ConfigManager& cfg) {
    if (m_fullRedrawNeeded) {
        drawAll(tft, state, cfg);
        if (m_qmVisible) {
            QuickMenuWidget::draw(tft, m_qmSelectedItem, state, cfg);
            QuickMenuWidget::resetDirtyCache(m_qmSelectedItem, state, cfg);
        }
        return;
    }

    updateDirtyWidgets(tft, state, cfg);

    if (m_qmVisible) {
        if (millis() - m_qmLastActivityMs >= QM_TIMEOUT_MS) {
            hideQuickMenu(tft, state, cfg);
        } else {
            QuickMenuWidget::updateDirty(tft, m_qmSelectedItem, state, cfg);
        }
    }
}

void HomeScreen::showQuickMenu(TFT_eSPI& tft,
                                const SystemState& state,
                                const ConfigManager& cfg,
                                uint8_t default_item) {
    m_qmVisible = true;
    m_qmSelectedItem = (default_item < QM_ITEM_COUNT) ? default_item : 0;
    m_qmLastActivityMs = millis();
    QuickMenuWidget::draw(tft, m_qmSelectedItem, state, cfg);
    QuickMenuWidget::resetDirtyCache(m_qmSelectedItem, state, cfg);
}

void HomeScreen::hideQuickMenu(TFT_eSPI& tft,
                                const SystemState& state,
                                const ConfigManager& cfg) {
    m_qmVisible = false;
    m_fullRedrawNeeded = true;
    drawAll(tft, state, cfg);
}

void HomeScreen::quickMenuSelectNext(TFT_eSPI& tft,
                                      const SystemState& state,
                                      const ConfigManager& cfg) {
    if (!m_qmVisible) return;

    const bool show_mode =
        (state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL);
    const uint8_t oldItem = m_qmSelectedItem;

    do {
        m_qmSelectedItem = static_cast<uint8_t>((m_qmSelectedItem + 1) % QM_ITEM_COUNT);
    } while (!show_mode && m_qmSelectedItem == 2);

    m_qmLastActivityMs = millis();
    QuickMenuWidget::refreshSelection(tft, oldItem, m_qmSelectedItem, state, cfg);
    QuickMenuWidget::resetDirtyCache(m_qmSelectedItem, state, cfg);
}

void HomeScreen::quickMenuSelectPrev(TFT_eSPI& tft,
                                      const SystemState& state,
                                      const ConfigManager& cfg) {
    if (!m_qmVisible) return;

    const bool show_mode =
        (state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL);
    const uint8_t oldItem = m_qmSelectedItem;

    do {
        m_qmSelectedItem = (m_qmSelectedItem == 0) ? static_cast<uint8_t>(QM_ITEM_COUNT - 1)
                                                   : static_cast<uint8_t>(m_qmSelectedItem - 1);
    } while (!show_mode && m_qmSelectedItem == 2);

    m_qmLastActivityMs = millis();
    QuickMenuWidget::refreshSelection(tft, oldItem, m_qmSelectedItem, state, cfg);
    QuickMenuWidget::resetDirtyCache(m_qmSelectedItem, state, cfg);
}

void HomeScreen::quickMenuAdjust(TFT_eSPI& tft,
                                  int8_t delta,
                                  SystemState& state,
                                  ConfigManager& cfg) {
    if (!m_qmVisible) return;
    m_qmLastActivityMs = millis();

    ConfigManager::Config conf = cfg.getConfig();
    bool changed = false;

    switch (m_qmSelectedItem) {
        case 0: {
            const int16_t newVal = static_cast<int16_t>(conf.target_temperature_c) + delta;
            conf.target_temperature_c = static_cast<uint8_t>(
                (newVal < 10) ? 10 : (newVal > 120) ? 120 : newVal);
            changed = true;
            break;
        }
        case 1: {
            const int16_t newVal = static_cast<int16_t>(conf.fuel_correction) + delta;
            conf.fuel_correction = static_cast<int16_t>(
                (newVal < -1000) ? -1000 : (newVal > 1000) ? 1000 : newVal);
            changed = true;
            break;
        }
        case 2:
            if (state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL) {
                state.postEnterAutoRequest();
            }
            break;
        case 3:
            break;
        default:
            break;
    }

    if (changed) {
        cfg.setConfig(conf);
    }

    QuickMenuWidget::refreshItem(tft, m_qmSelectedItem, m_qmSelectedItem, state, cfg);
    QuickMenuWidget::resetDirtyCache(m_qmSelectedItem, state, cfg);
}

bool HomeScreen::isQuickMenuVisible() const {
    return m_qmVisible;
}

void HomeScreen::drawAll(TFT_eSPI& tft,
                          const SystemState& state,
                          const ConfigManager& cfg) {
    const auto& pal  = PaletteManager::get(state.getAutomationState());
    const auto  hex  = state.getHeatExchangerState();
    const auto  room = state.getRoomClimateState();
    const auto  pump = state.getPumpState();
    const auto  mtr  = state.getMotorState();
    const auto  ign  = state.getIgnitorState();
    const auto& conf = cfg.getConfig();

    tft.fillScreen(CLR_BG);

    CoreWidget::drawAll(tft, hex.temperature_c, room.temperature_c,
                        room.humidity_percent, pal);
    ModeWidget::draw(tft, state.getAutomationState(), pal);
    PumpIndicator::draw(tft, pump.enabled, pal);
    PumpWidget::draw(tft, pump.cycle_period_ms, conf.pumpPerfomance, pump.enabled, pal);
    SpeedWidget::draw(tft, mtr.speed_index, pal);
    TimerWidget::draw(tft,
                      calcIgnitorElapsedSec(ign, cfg),
                      conf.ignitor_timeout_s,
                      pal);
    IgnitionIndicator::draw(tft, ign.enabled, pal);

    resetDirtyCache(state, cfg);
}

void HomeScreen::updateDirtyWidgets(TFT_eSPI& tft,
                                     const SystemState& state,
                                     const ConfigManager& cfg) {
    const auto newState = state.getAutomationState();
    const auto& pal  = PaletteManager::get(newState);
    const auto  hex  = state.getHeatExchangerState();
    const auto  room = state.getRoomClimateState();
    const auto  pump = state.getPumpState();
    const auto  mtr  = state.getMotorState();
    const auto  ign  = state.getIgnitorState();
    const auto& conf = cfg.getConfig();

    if (!m_qmVisible) {
        if (newState != m_lastState) {
            if (!m_qmVisible) {
                CoreWidget::drawAll(tft, hex.temperature_c, room.temperature_c,
                                    room.humidity_percent, pal);
                m_lastHexDisplay  = static_cast<int16_t>(hex.temperature_c);
                m_lastRoomDisplay = static_cast<int16_t>(room.temperature_c);
                m_lastHumDisplay  = static_cast<int16_t>(room.humidity_percent);
            }
            ModeWidget::draw(tft, newState, pal);
            m_lastState = newState;
        } else {
            const int16_t hexDisplay = static_cast<int16_t>(hex.temperature_c);
            if (hexDisplay != m_lastHexDisplay) {
                CoreWidget::updateHexTemp(tft, hex.temperature_c, pal);
                m_lastHexDisplay = hexDisplay;
            }

            const int16_t roomDisplay = static_cast<int16_t>(room.temperature_c);
            if (roomDisplay != m_lastRoomDisplay) {
                CoreWidget::updateRoomTemp(tft, room.temperature_c, pal);
                m_lastRoomDisplay = roomDisplay;
            }

            const int16_t humDisplay = static_cast<int16_t>(room.humidity_percent);
            if (humDisplay != m_lastHumDisplay) {
                CoreWidget::updateHumidity(tft, room.humidity_percent, pal);
                m_lastHumDisplay = humDisplay;
            }
        }
    } else {
        m_lastHexDisplay  = static_cast<int16_t>(hex.temperature_c);
        m_lastRoomDisplay = static_cast<int16_t>(room.temperature_c);
        m_lastHumDisplay  = static_cast<int16_t>(room.humidity_percent);
    }

    if (mtr.speed_index != m_lastSpeedIndex) {
        SpeedWidget::updateStep(tft, mtr.speed_index, pal);
        m_lastSpeedIndex = mtr.speed_index;
    }

    if (pump.enabled != m_lastPumpActive) {
        PumpIndicator::draw(tft, pump.enabled, pal);
        m_lastPumpActive = pump.enabled;
    }

    if (pump.cycle_period_ms != m_lastCyclePeriodMs) {
        PumpWidget::updateFlow(tft, pump.cycle_period_ms, conf.pumpPerfomance, pal);
        m_lastCyclePeriodMs = pump.cycle_period_ms;
    }

    const uint16_t timerSec = calcIgnitorElapsedSec(ign, cfg);
    if (timerSec != m_lastTimerSec || ign.enabled != m_lastIgnActive) {
        const uint16_t maxSec = conf.ignitor_timeout_s;
        TimerWidget::update(tft, timerSec, maxSec, pal);
        IgnitionIndicator::draw(tft, ign.enabled, pal);
        m_lastTimerSec  = timerSec;
        m_lastIgnActive = ign.enabled;
    }
}

void HomeScreen::resetDirtyCache(const SystemState& state, const ConfigManager& cfg) {
    const auto hex  = state.getHeatExchangerState();
    const auto room = state.getRoomClimateState();
    const auto pump = state.getPumpState();
    const auto mtr  = state.getMotorState();
    const auto ign  = state.getIgnitorState();

    m_lastState      = state.getAutomationState();
    m_lastHexDisplay  = static_cast<int16_t>(hex.temperature_c);
    m_lastRoomDisplay = static_cast<int16_t>(room.temperature_c);
    m_lastHumDisplay  = static_cast<int16_t>(room.humidity_percent);
    m_lastSpeedIndex = mtr.speed_index;
    m_lastCyclePeriodMs = pump.cycle_period_ms;
    m_lastPumpActive = pump.enabled;
    m_lastIgnActive  = ign.enabled;
    m_lastTimerSec   = calcIgnitorElapsedSec(ign, cfg);
    m_fullRedrawNeeded = false;
}

uint16_t HomeScreen::calcIgnitorElapsedSec(const SystemState::IgnitorState& ign,
                                            const ConfigManager& cfg) {
    if (!ign.enabled || ign.timeout_deadline_ms == 0) return 0u;

    const uint32_t now    = millis();
    const uint32_t maxMs  = static_cast<uint32_t>(cfg.getConfig().ignitor_timeout_s) * 1000u;

    if (now >= ign.timeout_deadline_ms) return static_cast<uint16_t>(maxMs / 1000u);

    const uint32_t remaining = ign.timeout_deadline_ms - now;
    const uint32_t elapsed   = (maxMs > remaining) ? (maxMs - remaining) : 0u;
    return static_cast<uint16_t>(elapsed / 1000u);
}

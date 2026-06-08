#include "display/screens/HomeScreen.h"

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdio.h>
#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"
#include "display/SpritePool.h"
#include "display/widgets/CoreWidget.h"
#include "display/widgets/ModeWidget.h"
#include "display/widgets/PumpIndicator.h"
#include "display/widgets/PumpWidget.h"
#include "display/widgets/SpeedWidget.h"
#include "display/widgets/TimerWidget.h"
#include "display/widgets/IgnitionIndicator.h"
#include "system/SystemState.h"
#include "system/ConfigManager.h"

// ─── Геометрия оверлея Быстрого меню ─────────────────────────────────────────
static constexpr int16_t QM_X      = 59;
static constexpr int16_t QM_Y      = 40;
static constexpr int16_t QM_W      = 202;
static constexpr int16_t QM_H      = 160;
static constexpr int16_t QM_RADIUS = 12;
static constexpr uint16_t QM_BG    = RGB565(0x1A, 0x1A, 0x1A); // #1a1a1a
static constexpr int16_t QM_ITEM_H = 36; // высота одного пункта
static constexpr int16_t QM_ITEM_PAD_X = 12;

// ─── Подписи пунктов Быстрого меню ───────────────────────────────────────────
static const char* const QM_LABELS[HomeScreen::QM_ITEM_COUNT] = {
    "\xD3\xF1\xF2\xE0\xE2\xEA\xE0",      // "Уставка"
    "\xCD\xE0\xF1\xEE\xF1",               // "Насос"
    "\xD0\xE5\xE6\xE8\xEC",               // "Режим"
    "\xCC\xE5\xED\xFE \xBB",              // "Меню »"
};

// ─────────────────────────────────────────────────────────────────────────────

HomeScreen::HomeScreen()
    : m_lastState(SystemState::AutomationState::STATE_IDLE)
    , m_lastHexDisplay(-999)
    , m_lastRoomDisplay(-999)
    , m_lastHumDisplay(-999)
    , m_lastSpeedIndex(0xFF)
    , m_lastTimerSec(0xFFFFu)
    , m_lastPulseHz(0xFFFFu)
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
    // Полная перерисовка только при принудительном флаге (onEnter, hideQuickMenu)
    if (m_fullRedrawNeeded) {
        drawAll(tft, state, cfg);
        if (m_qmVisible) {
            drawQuickMenuOverlay(tft, state, cfg);
        }
        return;
    }

    updateDirtyWidgets(tft, state, cfg);

    // Авто-закрытие Быстрого меню по таймауту
    if (m_qmVisible) {
        if (millis() - m_qmLastActivityMs >= QM_TIMEOUT_MS) {
            hideQuickMenu(tft, state, cfg);
        } else {
            drawQuickMenuOverlay(tft, state, cfg);
        }
    }
}

// ─── Быстрое меню ────────────────────────────────────────────────────────────

void HomeScreen::showQuickMenu(TFT_eSPI& tft,
                                const SystemState& state,
                                const ConfigManager& cfg,
                                uint8_t default_item) {
    m_qmVisible = true;
    // default_item: 0 при «Вверх», 3 при «Вниз» (с учётом сдвига в DisplayManager).
    m_qmSelectedItem = (default_item < QM_ITEM_COUNT) ? default_item : 0;
    m_qmLastActivityMs = millis();
    drawQuickMenuOverlay(tft, state, cfg);
}

void HomeScreen::hideQuickMenu(TFT_eSPI& tft,
                                const SystemState& state,
                                const ConfigManager& cfg) {
    m_qmVisible = false;
    // Восстанавливаем область под меню (перерисовываем затронутые виджеты)
    m_fullRedrawNeeded = true;
    drawAll(tft, state, cfg);
}

void HomeScreen::quickMenuSelectNext(TFT_eSPI& tft,
                                      const SystemState& state,
                                      const ConfigManager& cfg) {
    if (!m_qmVisible) return;
    // Пропуск пункта «Режим» (индекс 2), если не в ручном режиме.
    const bool show_mode =
        (state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL);
    do {
        m_qmSelectedItem = static_cast<uint8_t>((m_qmSelectedItem + 1) % QM_ITEM_COUNT);
    } while (!show_mode && m_qmSelectedItem == 2);
    m_qmLastActivityMs = millis();
    drawQuickMenuOverlay(tft, state, cfg);
}

void HomeScreen::quickMenuSelectPrev(TFT_eSPI& tft,
                                      const SystemState& state,
                                      const ConfigManager& cfg) {
    if (!m_qmVisible) return;
    const bool show_mode =
        (state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL);
    do {
        m_qmSelectedItem = (m_qmSelectedItem == 0) ? static_cast<uint8_t>(QM_ITEM_COUNT - 1)
                                                   : static_cast<uint8_t>(m_qmSelectedItem - 1);
    } while (!show_mode && m_qmSelectedItem == 2);
    m_qmLastActivityMs = millis();
    drawQuickMenuOverlay(tft, state, cfg);
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
        case 0: { // Уставка температуры
            const int16_t newVal = static_cast<int16_t>(conf.target_temperature_c) + delta;
            conf.target_temperature_c = static_cast<uint8_t>(
                (newVal < 10) ? 10 : (newVal > 120) ? 120 : newVal);
            changed = true;
            break;
        }
        case 1: { // Коррекция подачи топлива
            const int16_t newVal = static_cast<int16_t>(conf.fuel_correction) + delta;
            conf.fuel_correction = static_cast<int16_t>(
                (newVal < -1000) ? -1000 : (newVal > 1000) ? 1000 : newVal);
            changed = true;
            break;
        }
        case 2: // Режим AUTO: запрос в AutomationController (без прямой связи с UI)
            if (state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL) {
                state.postEnterAutoRequest();
            }
            break;
        case 3: // "Меню" — переключение на MainMenuScreen обрабатывается в DisplayManager
            break;
        default:
            break;
    }

    if (changed) {
        cfg.setConfig(conf);
    }

    drawQuickMenuOverlay(tft, state, cfg);
}

bool HomeScreen::isQuickMenuVisible() const {
    return m_qmVisible;
}

// ─── Приватные методы ─────────────────────────────────────────────────────────

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

    tft.fillScreen(pal.screenBg);

    CoreWidget::drawAll(tft, hex.temperature_c, room.temperature_c,
                        room.humidity_percent, pal);
    ModeWidget::draw(tft, state.getAutomationState(), pal);
    PumpIndicator::draw(tft, pump.enabled, pal);
    PumpWidget::draw(tft, pump.pulse_hz, pump.enabled, pal);
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

    // CoreWidget находится под оверлеем Быстрого меню — не перерисовываем
    // фон пока меню открыто, чтобы не было мерцания сквозь overlay.
    if (!m_qmVisible) {
        // Смена режима автоматики — новая палитра
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
        }else{
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

    if (pump.pulse_hz != m_lastPulseHz) {
        PumpWidget::updateFlow(tft, pump.pulse_hz, pal);
        m_lastPulseHz = pump.pulse_hz;
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
    m_lastPulseHz    = pump.pulse_hz;
    m_lastPumpActive = pump.enabled;
    m_lastIgnActive  = ign.enabled;
    m_lastTimerSec   = calcIgnitorElapsedSec(ign, cfg);
    m_fullRedrawNeeded = false;
}

void HomeScreen::drawQuickMenuOverlay(TFT_eSPI& tft,
                                       const SystemState& state,
                                       const ConfigManager& cfg) {
    // Фон панели оверлея (скруглённый прямоугольник, непрозрачный)
    tft.fillRoundRect(QM_X, QM_Y, QM_W, QM_H, QM_RADIUS, QM_BG);

    for (uint8_t i = 0; i < QM_ITEM_COUNT; ++i) {
        drawQuickMenuItem(tft, i, state, cfg);
    }
}

void HomeScreen::drawQuickMenuItem(TFT_eSPI& tft,
                                    uint8_t index,
                                    const SystemState& state,
                                    const ConfigManager& cfg) {
    const int16_t itemY = QM_Y + static_cast<int16_t>(index) * QM_ITEM_H + 4;
    const bool selected = (index == m_qmSelectedItem);

    // Подсветка выбранного пункта
    if (selected) {
        const uint16_t hlColor = RGB565(0x30, 0x30, 0x38);
        tft.fillRoundRect(QM_X + 4, itemY, QM_W - 8, QM_ITEM_H - 4, 6, hlColor);
    }

    // Метка пункта (слева)
    
    tft.loadFont(smooth_font::small);
    tft.setTextDatum(ML_DATUM);
    const uint16_t labelColor = selected ? RGB565(0xFF, 0xFF, 0xFF) : RGB565(0xA0, 0xA0, 0xA0);
    tft.setTextColor(labelColor, selected ? RGB565(0x30, 0x30, 0x38) : QM_BG);
    tft.drawString(QM_LABELS[index], QM_X + QM_ITEM_PAD_X, itemY + QM_ITEM_H / 2 - 2);
    tft.unloadFont();

    // Значение пункта (справа)
    char valBuf[12] = "";
    const auto& conf = cfg.getConfig();
    switch (index) {
        case 0: snprintf(valBuf, sizeof(valBuf), "%d C", conf.target_temperature_c); break;
        case 1: snprintf(valBuf, sizeof(valBuf), "%d",   conf.fuel_correction);      break;
        case 2: {
            const auto s = state.getAutomationState();
            const bool isAuto = (s == SystemState::AutomationState::STATE_AUTO_START ||
                                 s == SystemState::AutomationState::STATE_AUTO_TSTAT);
            snprintf(valBuf, sizeof(valBuf), isAuto ? "ABTO" : "\xD0\xD3"); // "АВТО" / "РУ"
            break;
        }
        case 3: break; // "Меню" — нет значения
    }

    if (valBuf[0] != '\0') {
        tft.loadFont(smooth_font::def);
        tft.setTextDatum(MR_DATUM);
        tft.setTextColor(selected ? RGB565(0xFF, 0xFF, 0xFF) : RGB565(0xDF, 0xDF, 0xDF),
                         selected ? RGB565(0x30, 0x30, 0x38) : QM_BG);
        tft.drawString(valBuf, QM_X + QM_W - QM_ITEM_PAD_X, itemY + QM_ITEM_H / 2 - 2);
        tft.unloadFont();
    }

    tft.setTextFont(1);
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

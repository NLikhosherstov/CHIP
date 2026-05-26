#pragma once
#include <stdint.h>
#include "display/BaseScreen.h"
#include "system/SystemState.h"

class TFT_eSPI;
class ConfigManager;

// Главный экран устройства.
//
// Содержит:
//   — Семь виджетов (CoreWidget, ModeWidget, PumpWidget, SpeedWidget,
//                    TimerWidget, IgnitionIndicator)
//   — Встроенный модальный оверлей Быстрого меню (QuickMenu).
//
// Быстрое меню НЕ является отдельным экраном: оно рисуется поверх
// главного экрана, фоновые виджеты продолжают обновляться в tick().
class HomeScreen : public BaseScreen {
public:
    HomeScreen();

    // ── BaseScreen ────────────────────────────────────────────────────────
    void onEnter(TFT_eSPI& tft,
                 const SystemState& state,
                 const ConfigManager& cfg) override;

    void onExit() override;

    void tick(TFT_eSPI& tft,
              const SystemState& state,
              const ConfigManager& cfg) override;

    // ── Управление Быстрым меню ──────────────────────────────────────────
    // default_item: индекс выбранного пункта при открытии (0 — «Вверх», 3 — «Вниз»).
    void showQuickMenu(TFT_eSPI& tft,
                       const SystemState& state,
                       const ConfigManager& cfg,
                       uint8_t default_item = 0);

    void hideQuickMenu(TFT_eSPI& tft,
                       const SystemState& state,
                       const ConfigManager& cfg);

    // Перемещение курсора по пунктам меню (кнопки Вверх/Вниз)
    void quickMenuSelectNext(TFT_eSPI& tft,
                             const SystemState& state,
                             const ConfigManager& cfg);

    void quickMenuSelectPrev(TFT_eSPI& tft,
                             const SystemState& state,
                             const ConfigManager& cfg);

    // Изменение значения выбранного пункта (поворот энкодера)
    void quickMenuAdjust(TFT_eSPI& tft,
                         int8_t delta,
                         SystemState& state,
                         ConfigManager& cfg);

    bool isQuickMenuVisible() const;
    uint8_t quickMenuSelectedItem() const { return m_qmSelectedItem; }  // для Click в DisplayManager

private:
    // ── Кеш для отслеживания изменений (dirty tracking) ──────────────────
    SystemState::AutomationState m_lastState;
    int16_t  m_lastHexDisplay;
    int16_t  m_lastRoomDisplay;
    int16_t  m_lastHumDisplay;
    uint8_t  m_lastSpeedIndex;
    uint16_t m_lastTimerSec;
    uint16_t m_lastPulseHz;
    bool     m_lastIgnActive;
    bool     m_lastPumpActive;
    bool     m_fullRedrawNeeded;

public:
    // ── Константы Быстрого меню (public для использования в .cpp) ────────
    static constexpr uint8_t  QM_ITEM_COUNT  = 4;
    static constexpr uint32_t QM_TIMEOUT_MS  = 5000; // авто-закрытие после 5 с бездействия

private:

    bool     m_qmVisible;
    uint8_t  m_qmSelectedItem;     // 0..QM_ITEM_COUNT-1
    uint32_t m_qmLastActivityMs;   // millis() последнего взаимодействия

    // ── Приватные хелперы ─────────────────────────────────────────────────

    // Полная перерисовка всех виджетов и сброс dirty-флагов
    void drawAll(TFT_eSPI& tft,
                 const SystemState& state,
                 const ConfigManager& cfg);

    // Частичное обновление только изменившихся виджетов
    void updateDirtyWidgets(TFT_eSPI& tft,
                            const SystemState& state,
                            const ConfigManager& cfg);

    // Сброс кеша к текущим значениям состояния
    void resetDirtyCache(const SystemState& state, const ConfigManager& cfg);

    // Отрисовка оверлея Быстрого меню (поверх уже нарисованного фона)
    void drawQuickMenuOverlay(TFT_eSPI& tft,
                              const SystemState& state,
                              const ConfigManager& cfg);

    // Перерисовка одного пункта меню
    void drawQuickMenuItem(TFT_eSPI& tft,
                           uint8_t index,
                           const SystemState& state,
                           const ConfigManager& cfg);

    // Вычисление прошедших секунд работы свечи
    static uint16_t calcIgnitorElapsedSec(const SystemState::IgnitorState& ign,
                                          const ConfigManager& cfg);
};

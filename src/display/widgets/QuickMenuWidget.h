#pragma once

#include <stddef.h>
#include <stdint.h>

#include "system/ConfigManager.h"
#include "system/SystemState.h"

class TFT_eSPI;

// Модальный оверлей «Быстрое меню» на главном экране.
struct QuickMenuWidget {
    static constexpr uint8_t ITEM_COUNT = 4;

    static constexpr int16_t X      = 70;
    static constexpr int16_t Y      = 45;
    static constexpr int16_t W      = 180;
    static constexpr int16_t H      = 150;
    static constexpr int16_t RADIUS = 12;

    // Полная отрисовка панели и всех пунктов.
    static void draw(TFT_eSPI& tft, uint8_t selectedItem,
                     const SystemState& state, const ConfigManager& cfg);

    // Перерисовка одного пункта (для частичного обновления).
    static void drawItem(TFT_eSPI& tft, uint8_t index, uint8_t selectedItem,
                         const SystemState& state, const ConfigManager& cfg);

    // Смена выделения: старый и новый пункт.
    static void refreshSelection(TFT_eSPI& tft, uint8_t oldIndex, uint8_t newIndex,
                                 const SystemState& state, const ConfigManager& cfg);

    // Перерисовка значения выбранного пункта.
    static void refreshItem(TFT_eSPI& tft, uint8_t index, uint8_t selectedItem,
                            const SystemState& state, const ConfigManager& cfg);

    // Частичное обновление в tick(): только если изменилось значение выбранного пункта.
    static void updateDirty(TFT_eSPI& tft, uint8_t selectedItem,
                            const SystemState& state, const ConfigManager& cfg);

    // Сброс кеша dirty-отслеживания к текущим значениям.
    static void resetDirtyCache(uint8_t selectedItem,
                                const SystemState& state, const ConfigManager& cfg);

private:
    static uint8_t m_lastTargetTemp;
    static int16_t m_lastFuelCorrection;
    static SystemState::AutomationState m_lastAutomationState;

    static void formatItemValue(uint8_t index, const SystemState& state,
                                const ConfigManager::Config& conf,
                                char* buf, size_t bufSize);
    static bool selectedItemValueChanged(uint8_t selectedItem,
                                         const SystemState& state,
                                         const ConfigManager& cfg);
    static void syncDirtyCache(uint8_t selectedItem,
                               const SystemState& state, const ConfigManager& cfg);
};

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "display/BaseScreen.h"
#include "display/UiInput.h"
#include "system/ConfigManager.h"

// Экран главного меню: 2 страницы × 2 колонки × 4 пункта (16 параметров конфига).
class MainMenuScreen : public BaseScreen {
public:
    static constexpr uint8_t ITEM_COUNT  = 16;
    static constexpr uint8_t PAGE_COUNT  = 2;

    MainMenuScreen() = default;

    void onEnter(TFT_eSPI& tft,
                 const SystemState& state,
                 const ConfigManager& cfg) override;

    void onExit() override;

    void tick(TFT_eSPI& tft,
              const SystemState& state,
              const ConfigManager& cfg) override;

    bool navigate(UiInput action);
    void refreshSelection(TFT_eSPI& tft, uint8_t oldIndex, uint8_t newIndex,
                          const ConfigManager& cfg);
    void adjustValue(int8_t delta, ConfigManager& cfg);
    void refreshItem(TFT_eSPI& tft, uint8_t index, const ConfigManager& cfg);

    uint8_t selectedIndex() const { return m_selectedIndex; }

private:
    uint8_t m_selectedIndex = 0;
    uint8_t m_currentPage   = 0;

    static uint8_t itemPage(uint8_t index);
    static uint8_t itemCol(uint8_t index);
    static uint8_t itemRow(uint8_t index);
    static uint8_t indexFrom(uint8_t page, uint8_t col, uint8_t row);

    void drawHeader(TFT_eSPI& tft);
    void drawPage(TFT_eSPI& tft, const ConfigManager& cfg);
    void drawPageItems(TFT_eSPI& tft, uint8_t page, const ConfigManager& cfg);
    void drawItem(TFT_eSPI& tft, uint8_t index, const ConfigManager& cfg);
    void drawPageIndicator(TFT_eSPI& tft);
    void formatItemValue(uint8_t index, const ConfigManager::Config& conf,
                         char* buf, size_t bufSize);
};

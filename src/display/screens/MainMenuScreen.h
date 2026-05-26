#pragma once
#include <stdint.h>
#include "display/BaseScreen.h"

// Экран Главного меню (глубокие настройки).
// Текстовый список параметров и значений.
// Реализация в разработке — текущая версия является каркасом.
class MainMenuScreen : public BaseScreen {
public:
    MainMenuScreen() = default;

    void onEnter(TFT_eSPI& tft,
                 const SystemState& state,
                 const ConfigManager& cfg) override;

    void onExit() override;

    void tick(TFT_eSPI& tft,
              const SystemState& state,
              const ConfigManager& cfg) override;

    // Навигация (делегируется из DisplayManager по мере реализации)
    void selectNext();
    void selectPrev();
    void adjustValue(int8_t delta, ConfigManager& cfg);

    // Перерисовка после навигации/правки из DisplayManager.
    void refresh(TFT_eSPI& tft, const ConfigManager& cfg);

    uint8_t selectedItem() const { return m_selectedItem; }

private:
    uint8_t m_selectedItem = 0;

    void drawAll(TFT_eSPI& tft, const ConfigManager& cfg);
    void drawItem(TFT_eSPI& tft, uint8_t index, const ConfigManager& cfg);
};

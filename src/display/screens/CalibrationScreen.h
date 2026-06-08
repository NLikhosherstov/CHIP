#pragma once

#include <stdint.h>

#include "display/BaseScreen.h"

class ConfigManager;
class TFT_eSprite;

// Полноэкранная калибровка резистивной клавиатуры (5 кнопок).
class CalibrationScreen : public BaseScreen {
public:
    static constexpr uint8_t STEP_COUNT = 5;

    CalibrationScreen() = default;

    void onEnter(TFT_eSPI& tft,
                 const SystemState& state,
                 const ConfigManager& cfg) override;

    void onExit() override;

    void tick(TFT_eSPI& tft,
              const SystemState& state,
              const ConfigManager& cfg) override;

    void loadFromConfig(const ConfigManager& cfg);
    void advanceStep();
    bool saveToConfig(ConfigManager& cfg) const;

    void refresh(TFT_eSPI& tft);

    uint8_t activeStep() const { return m_activeStep; }

private:
    void drawAll(TFT_eSPI& tft);
    void drawTitle(TFT_eSPI& tft);
    void drawCenterWidget(TFT_eSPI& tft);
    void drawBottomRow(TFT_eSPI& tft);
    void drawBottomColumn(TFT_eSPI& tft, uint8_t index);

    static void drawLabelValue(TFT_eSprite& spr,
                               int16_t x,
                               int16_t y,
                               const char* label,
                               const char* value,
                               int16_t valueY,
                               uint16_t valColor);

    uint16_t m_draft[STEP_COUNT]{};
    uint16_t m_lastDrawn[STEP_COUNT]{};
    uint8_t m_activeStep = 0;
    bool m_fullRedrawNeeded = true;
};

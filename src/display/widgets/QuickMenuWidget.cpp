#include "display/widgets/QuickMenuWidget.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"
#include "system/ConfigManager.h"

namespace {

static constexpr int16_t  QM_ITEM_H = 36;
static constexpr int16_t  QM_ITEM_PAD_X = 14;

static const char* const QM_LABELS[QuickMenuWidget::ITEM_COUNT] = {
    "Уст. Темп.",
    "Насос",
    "Режим АВТО",
    "Меню",
};

}  // namespace

uint8_t QuickMenuWidget::m_lastTargetTemp = 0;
int16_t QuickMenuWidget::m_lastFuelCorrection = 0;
SystemState::AutomationState QuickMenuWidget::m_lastAutomationState =
    SystemState::AutomationState::STATE_IDLE;

void QuickMenuWidget::draw(TFT_eSPI& tft, uint8_t selectedItem,
                            const SystemState& state, const ConfigManager& cfg) {
    tft.fillRoundRect(X, Y, W, H, RADIUS, CLR_ICON_INACTIVE);

    for (uint8_t i = 0; i < ITEM_COUNT; ++i) {
        drawItem(tft, i, selectedItem, state, cfg);
    }
}

void QuickMenuWidget::drawItem(TFT_eSPI& tft, uint8_t index, uint8_t selectedItem,
                                const SystemState& state, const ConfigManager& cfg) {
    const int16_t itemY = Y + static_cast<int16_t>(index) * QM_ITEM_H + 4;
    const bool selected = (index == selectedItem);

    const uint16_t rowBg = selected ? CLR_SELECTION : CLR_ICON_INACTIVE;
    tft.fillRoundRect(X + 4, itemY, W - 8, QM_ITEM_H - 4, 6, rowBg);

    tft.loadFont(smooth_font::small);
    tft.setTextDatum(ML_DATUM);
    const uint16_t labelColor = selected ? CLR_VALUE : CLR_LABEL;
    tft.setTextColor(labelColor, rowBg);
    tft.drawString(QM_LABELS[index], X + QM_ITEM_PAD_X, itemY + QM_ITEM_H / 2 - 2);
    tft.unloadFont();

    char valBuf[12] = "";
    formatItemValue(index, state, cfg.getConfig(), valBuf, sizeof(valBuf));

    if (valBuf[0] != '\0') {
        tft.loadFont(smooth_font::def);
        tft.setTextDatum(MR_DATUM);
        tft.setTextColor(selected ? TFT_WHITE : CLR_VALUE,
                         rowBg);
        tft.drawString(valBuf, X + W - QM_ITEM_PAD_X, itemY + QM_ITEM_H / 2 - 2);
        tft.unloadFont();
    }

    tft.setTextFont(1);
}

void QuickMenuWidget::refreshSelection(TFT_eSPI& tft, uint8_t oldIndex, uint8_t newIndex,
                                        const SystemState& state, const ConfigManager& cfg) {
    if (oldIndex != newIndex) {
        drawItem(tft, oldIndex, newIndex, state, cfg);
        drawItem(tft, newIndex, newIndex, state, cfg);
    }
}

void QuickMenuWidget::refreshItem(TFT_eSPI& tft, uint8_t index, uint8_t selectedItem,
                                   const SystemState& state, const ConfigManager& cfg) {
    drawItem(tft, index, selectedItem, state, cfg);
}

void QuickMenuWidget::updateDirty(TFT_eSPI& tft, uint8_t selectedItem,
                                   const SystemState& state, const ConfigManager& cfg) {
    if (!selectedItemValueChanged(selectedItem, state, cfg)) {
        return;
    }

    refreshItem(tft, selectedItem, selectedItem, state, cfg);
    syncDirtyCache(selectedItem, state, cfg);
}

void QuickMenuWidget::resetDirtyCache(uint8_t selectedItem,
                                       const SystemState& state, const ConfigManager& cfg) {
    syncDirtyCache(selectedItem, state, cfg);
}

void QuickMenuWidget::formatItemValue(uint8_t index, const SystemState& state,
                                       const ConfigManager::Config& conf,
                                       char* buf, size_t bufSize) {
    switch (index) {
        case 0:
            snprintf(buf, bufSize, "%d°С", conf.target_temperature_c);
            break;
        case 1:
            snprintf(buf, bufSize, "%d", conf.fuel_correction);
            break;
        case 2: {
            // const auto s = state.getAutomationState();
            // const bool isAuto = (s == SystemState::AutomationState::STATE_AUTO_START ||
            //                      s == SystemState::AutomationState::STATE_AUTO_TSTAT);
            // snprintf(buf, bufSize, isAuto ? "Aвто" : "Ручной");
            // break;
        }
        case 3:
        default:
            buf[0] = '\0';
            break;
    }
}

bool QuickMenuWidget::selectedItemValueChanged(uint8_t selectedItem,
                                                const SystemState& state,
                                                const ConfigManager& cfg) {
    const auto& conf = cfg.getConfig();

    switch (selectedItem) {
        case 0:
            return conf.target_temperature_c != m_lastTargetTemp;
        case 1:
            return conf.fuel_correction != m_lastFuelCorrection;
        case 2:
            return state.getAutomationState() != m_lastAutomationState;
        default:
            return false;
    }
}

void QuickMenuWidget::syncDirtyCache(uint8_t selectedItem,
                                      const SystemState& state, const ConfigManager& cfg) {
    const auto& conf = cfg.getConfig();

    m_lastTargetTemp      = conf.target_temperature_c;
    m_lastFuelCorrection  = conf.fuel_correction;
    m_lastAutomationState = state.getAutomationState();

    (void)selectedItem;
}

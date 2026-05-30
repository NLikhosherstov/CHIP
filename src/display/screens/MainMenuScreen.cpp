#include "display/screens/MainMenuScreen.h"

#include <TFT_eSPI.h>
#include <stdio.h>

#include "display/assets/fonts.h"
#include "display/PaletteRGB565.h"
#include "system/SystemState.h"
#include "system/ConfigManager.h"

// Параметры меню: метка + доступ к значению через ConfigManager::Config
struct MenuItem {
    const char* label;
};

static constexpr uint8_t ITEM_COUNT = 4;

static const MenuItem ITEMS[ITEM_COUNT] = {
    { "\xD6\xE5\xEB. \xF2\xE5\xEC\xEF\xE5\xF0\xE0\xF2\xF3\xF0\xE0" }, // "Цел. температура"
    { "\xD1\xEA\xEE\xF0. \xED\xE0\xF1\xEE\xF1\xE0"                   }, // "Скор. насоса"
    { "\xD1\xEA\xEE\xF0. \xEC\xEE\xF2\xEE\xF0\xE0"                   }, // "Скор. мотора"
    { "\xD2\xE0\xE9\xEC\xE0\xF3\xF2 \xF1\xE2\xE5\xF7\xE8"            }, // "Таймаут свечи"
};

static constexpr uint16_t BG_COLOR       = RGB565(0x00, 0x0B, 0x16);
static constexpr uint16_t HEADER_COLOR   = RGB565(0xDF, 0xDF, 0xDF);
static constexpr uint16_t LABEL_COLOR    = RGB565(0x80, 0x80, 0x80);
static constexpr uint16_t VALUE_COLOR    = RGB565(0xDF, 0xDF, 0xDF);
static constexpr uint16_t SELECT_BG      = RGB565(0x1A, 0x20, 0x2D);
static constexpr int16_t  ITEM_H         = 44;
static constexpr int16_t  HEADER_H       = 28;

void MainMenuScreen::onEnter(TFT_eSPI& tft,
                              const SystemState& /*state*/,
                              const ConfigManager& cfg) {
    m_selectedItem = 0;
    drawAll(tft, cfg);
}

void MainMenuScreen::onExit() {
    m_selectedItem = 0;
}

void MainMenuScreen::tick(TFT_eSPI& tft,
                           const SystemState& /*state*/,
                           const ConfigManager& /*cfg*/) {
    // Главное меню статично — перерисовка только по событиям навигации
    (void)tft;
}

void MainMenuScreen::selectNext() {
    if (m_selectedItem < ITEM_COUNT - 1) ++m_selectedItem;
}

void MainMenuScreen::selectPrev() {
    if (m_selectedItem > 0) --m_selectedItem;
}

void MainMenuScreen::refresh(TFT_eSPI& tft, const ConfigManager& cfg) {
    drawAll(tft, cfg);
}

void MainMenuScreen::adjustValue(int8_t delta, ConfigManager& cfg) {
    ConfigManager::Config conf = cfg.getConfig();
    switch (m_selectedItem) {
        case 0: {
            const int16_t v = static_cast<int16_t>(conf.target_temperature_c) + delta;
            conf.target_temperature_c = static_cast<uint8_t>((v < 10) ? 10 : (v > 120) ? 120 : v);
            break;
        }
        case 1: {
            const int16_t v = static_cast<int16_t>(conf.fuel_correction) + delta;
            conf.fuel_correction = static_cast<int16_t>((v < -1000) ? -1000 : (v > 1000) ? 1000 : v);
            break;
        }
        case 2: {
            const int16_t v = static_cast<int16_t>(conf.motor_PWM_1) + delta;
            conf.motor_PWM_1 = static_cast<uint8_t>((v < 0) ? 0 : (v > 255) ? 255 : v);
            break;
        }
        case 3: {
            const int32_t v = static_cast<int32_t>(conf.ignitor_timeout_s) + delta;
            conf.ignitor_timeout_s = static_cast<uint16_t>((v < 10) ? 10 : (v > 600) ? 600 : v);
            break;
        }
        default: break;
    }
    cfg.setConfig(conf);
}

// ─── Приватные ────────────────────────────────────────────────────────────────

void MainMenuScreen::drawAll(TFT_eSPI& tft, const ConfigManager& cfg) {
    tft.fillScreen(BG_COLOR);

    // Заголовок
    tft.setFreeFont(Font_default);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(HEADER_COLOR, BG_COLOR);
    tft.drawString("\xCD\xE0\xF1\xF2\xF0\xEE\xE9\xEA\xE8", 8, 4); // "Настройки"
    tft.drawFastHLine(0, HEADER_H, 320, LABEL_COLOR);

    for (uint8_t i = 0; i < ITEM_COUNT; ++i) {
        drawItem(tft, i, cfg);
    }

    tft.setTextFont(1);
}

void MainMenuScreen::drawItem(TFT_eSPI& tft,
                               uint8_t index,
                               const ConfigManager& cfg) {
    const int16_t itemY     = HEADER_H + 2 + static_cast<int16_t>(index) * ITEM_H;
    const bool    selected  = (index == m_selectedItem);
    const uint16_t rowBg    = selected ? SELECT_BG : BG_COLOR;

    tft.fillRect(0, itemY, 320, ITEM_H, rowBg);

    // Метка параметра
    tft.setFreeFont(Font_small);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(LABEL_COLOR, rowBg);
    tft.drawString(ITEMS[index].label, 8, itemY + ITEM_H / 4);

    // Значение
    char valBuf[16] = "";
    const auto& conf = cfg.getConfig();
    switch (index) {
        case 0: snprintf(valBuf, sizeof(valBuf), "%d C",   conf.target_temperature_c);     break;
        case 1: snprintf(valBuf, sizeof(valBuf), "%d",     conf.fuel_correction);          break;
        case 2: snprintf(valBuf, sizeof(valBuf), "%d",     conf.motor_PWM_1);              break;
        case 3: snprintf(valBuf, sizeof(valBuf), "%lu c",
                         static_cast<unsigned long>(conf.ignitor_timeout_s));              break;
        default: break;
    }

    tft.setFreeFont(Font_default);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(VALUE_COLOR, rowBg);
    tft.drawString(valBuf, 8, itemY + ITEM_H * 3 / 4);

    // Курсор выбора
    if (selected) {
        tft.fillTriangle(310, itemY + 4,
                         318, itemY + ITEM_H / 2,
                         310, itemY + ITEM_H - 4,
                         VALUE_COLOR);
    }

    tft.setTextFont(1);
}

#include "display/screens/MainMenuScreen.h"

#include <TFT_eSPI.h>
#include <stdio.h>
#include <string.h>

#include "display/IconDraw.h"
#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/assets/fonts.h"
#include "display/assets/icons.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

static constexpr int16_t HEADER_H         = 19;
static constexpr int16_t COL_L_X          = 5;
static constexpr int16_t COL_R_X          = 160;
static constexpr int16_t ITEM_Y0          = 24;
static constexpr int16_t ROW_STEP         = SpritePool::XL_H;
static constexpr int16_t ITEM_PAD_X       = 6;
static constexpr int16_t ITEM_LABEL_Y     = 10;
static constexpr int16_t ITEM_VALUE_Y     = 27;

static constexpr int16_t PAGE_IND_X       = 315;
static constexpr int16_t PAGE_IND_BAR_W   = 2;
static constexpr int16_t PAGE_IND_BAR_H   = 23;
static constexpr int16_t PAGE_IND_GAP       = 4;

static const char* const LABELS[MainMenuScreen::ITEM_COUNT] = {
    "Коррекция подачи",
    "Встроенный насос",
    "Производ. насоса",
    "Макс. Расход",
    "Целевая температура",
    "Охлаждение ТО до",
    "Гистерезис",
    "Таймер свечи",
    "Скорость мотора 1",
    "Скорость мотора 2",
    "Скорость мотора 3",
    "Скорость мотора 4",
    "Интервал импульса 1",
    "Интервал импульса 2",
    "Интервал импульса 3",
    "Интервал импульса 4",
};

int16_t pwmToPercent(uint8_t pwm) {
    return static_cast<int16_t>((static_cast<uint16_t>(pwm) * 100u + 127u) / 255u);
}

void getItemScreenPos(uint8_t index, int16_t& outX, int16_t& outY) {
    const uint8_t col = (index / 4) & 1;
    const uint8_t row = index % 4;
    outX = (col == 0) ? COL_L_X : COL_R_X;
    outY = ITEM_Y0 + static_cast<int16_t>(row) * ROW_STEP;
}

}  // namespace

uint8_t MainMenuScreen::itemPage(uint8_t index) {
    return index / 8;
}

uint8_t MainMenuScreen::itemCol(uint8_t index) {
    return (index / 4) & 1;
}

uint8_t MainMenuScreen::itemRow(uint8_t index) {
    return index % 4;
}

uint8_t MainMenuScreen::indexFrom(uint8_t page, uint8_t col, uint8_t row) {
    return static_cast<uint8_t>(page * 8 + col * 4 + row);
}

void MainMenuScreen::onEnter(TFT_eSPI& tft,
                              const SystemState& /*state*/,
                              const ConfigManager& cfg) {
    m_selectedIndex = 0;
    m_currentPage   = 0;
    SpritePool::init(tft);
    tft.fillScreen(CLR_BG);
    drawHeader(tft);
    drawPage(tft, cfg);
}

void MainMenuScreen::onExit() {
    m_selectedIndex = 0;
    m_currentPage   = 0;
    SpritePool::shutdown();
}

void MainMenuScreen::tick(TFT_eSPI& tft,
                           const SystemState& /*state*/,
                           const ConfigManager& /*cfg*/) {
    (void)tft;
}

bool MainMenuScreen::navigate(EventType action) {
    const uint8_t oldIndex = m_selectedIndex;
    uint8_t page = itemPage(m_selectedIndex);
    uint8_t col  = itemCol(m_selectedIndex);
    uint8_t row  = itemRow(m_selectedIndex);

    switch (action) {
        case EventType::Up:
            if (row == 0) {
                row = 3;
                page ^= 1;
            } else {
                --row;
            }
            break;
        case EventType::Down:
            if (row == 3) {
                row = 0;
                page ^= 1;
            } else {
                ++row;
            }
            break;
        case EventType::Left:
            if (col == 0) {
                return false;
            }
            col = 0;
            break;
        case EventType::Right:
            if (col == 1) {
                return false;
            }
            col = 1;
            break;
        default:
            return false;
    }

    m_selectedIndex = indexFrom(page, col, row);
    m_currentPage   = page;
    return m_selectedIndex != oldIndex;
}

void MainMenuScreen::refreshSelection(TFT_eSPI& tft, uint8_t oldIndex, uint8_t newIndex,
                                       const ConfigManager& cfg) {
    const uint8_t oldPage = itemPage(oldIndex);
    const uint8_t newPage = itemPage(newIndex);

    if (oldPage != newPage) {
        drawPage(tft, cfg);
        return;
    }

    drawItem(tft, oldIndex, cfg);
    drawItem(tft, newIndex, cfg);
}

void MainMenuScreen::refreshItem(TFT_eSPI& tft, uint8_t index, const ConfigManager& cfg) {
    drawItem(tft, index, cfg);
}

void MainMenuScreen::adjustValue(int8_t delta, ConfigManager& cfg) {
    if (delta == 0) {
        return;
    }

    ConfigManager::Config conf = cfg.getConfig();

    switch (m_selectedIndex) {
        case 0: {
            const int16_t v = static_cast<int16_t>(conf.fuel_correction) + delta;
            conf.fuel_correction = static_cast<int16_t>(
                (v < -1000) ? -1000 : (v > 1000) ? 1000 : v);
            break;
        }
        case 1:
            conf.embededPump = !conf.embededPump;
            break;
        case 2: {
            const int16_t v = static_cast<int16_t>(conf.pumpPerfomance) + delta;
            conf.pumpPerfomance = static_cast<uint16_t>(
                (v < 10) ? 10 : (v > 1000) ? 1000 : v);
            break;
        }
        case 3: {
            const int16_t v = static_cast<int16_t>(conf.max_fuel_flow_lph_x10) + delta;
            conf.max_fuel_flow_lph_x10 = static_cast<uint16_t>(
                (v < 1) ? 1 : (v > 100) ? 100 : v);
            break;
        }
        case 4: {
            const int16_t v = static_cast<int16_t>(conf.target_temperature_c) + delta;
            conf.target_temperature_c = static_cast<uint8_t>(
                (v < 10) ? 10 : (v > 120) ? 120 : v);
            break;
        }
        case 5: {
            const int16_t v = static_cast<int16_t>(conf.cooling_target_c) + delta;
            conf.cooling_target_c = static_cast<uint8_t>(
                (v < 30) ? 30 : (v > 90) ? 90 : v);
            break;
        }
        case 6: {
            const int16_t v = static_cast<int16_t>(conf.temperature_hysteresis_c) + delta;
            conf.temperature_hysteresis_c = static_cast<uint8_t>(
                (v < 1) ? 1 : (v > 20) ? 20 : v);
            break;
        }
        case 7: {
            const int32_t v = static_cast<int32_t>(conf.ignitor_timeout_s) + delta;
            conf.ignitor_timeout_s = static_cast<uint16_t>(
                (v < 10) ? 10 : (v > 600) ? 600 : v);
            break;
        }
        case 8: {
            const int16_t v = static_cast<int16_t>(conf.motor_PWM_1) + delta;
            conf.motor_PWM_1 = static_cast<uint8_t>((v < 0) ? 0 : (v > 255) ? 255 : v);
            break;
        }
        case 9: {
            const int16_t v = static_cast<int16_t>(conf.motor_PWM_2) + delta;
            conf.motor_PWM_2 = static_cast<uint8_t>((v < 0) ? 0 : (v > 255) ? 255 : v);
            break;
        }
        case 10: {
            const int16_t v = static_cast<int16_t>(conf.motor_PWM_3) + delta;
            conf.motor_PWM_3 = static_cast<uint8_t>((v < 0) ? 0 : (v > 255) ? 255 : v);
            break;
        }
        case 11: {
            const int16_t v = static_cast<int16_t>(conf.motor_PWM_4) + delta;
            conf.motor_PWM_4 = static_cast<uint8_t>((v < 0) ? 0 : (v > 255) ? 255 : v);
            break;
        }
        case 12: {
            const int16_t v = static_cast<int16_t>(conf.pump_pulse_1) + delta;
            conf.pump_pulse_1 = static_cast<uint16_t>(
                (v < 10) ? 10 : (v > 1000) ? 1000 : v);
            break;
        }
        case 13: {
            const int16_t v = static_cast<int16_t>(conf.pump_pulse_2) + delta;
            conf.pump_pulse_2 = static_cast<uint16_t>(
                (v < 10) ? 10 : (v > 1000) ? 1000 : v);
            break;
        }
        case 14: {
            const int16_t v = static_cast<int16_t>(conf.pump_pulse_3) + delta;
            conf.pump_pulse_3 = static_cast<uint16_t>(
                (v < 10) ? 10 : (v > 1000) ? 1000 : v);
            break;
        }
        case 15: {
            const int16_t v = static_cast<int16_t>(conf.pump_pulse_4) + delta;
            conf.pump_pulse_4 = static_cast<uint16_t>(
                (v < 10) ? 10 : (v > 1000) ? 1000 : v);
            break;
        }
        default:
            break;
    }

    cfg.setConfig(conf);
}

void MainMenuScreen::formatItemValue(uint8_t index,
                                      const ConfigManager::Config& conf,
                                      char* buf,
                                      size_t bufSize) {
    switch (index) {
        case 0:
            snprintf(buf, bufSize, "%d%%", conf.fuel_correction);
            break;
        case 1:
            snprintf(buf, bufSize, "%s", conf.embededPump ? "Да" : "Нет");
            break;
        case 2: {
            const float ml = static_cast<float>(conf.pumpPerfomance) / 10.0f;
            dtostrf(ml, 0, 1, buf);
            const size_t len = strlen(buf);
            if (len + 9 < bufSize) {
                snprintf(buf + len, bufSize - len, " мл/100");
            }
            break;
        }
        case 3: {
            const float lph = static_cast<float>(conf.max_fuel_flow_lph_x10) / 10.0f;
            dtostrf(lph, 0, 1, buf);
            const size_t len = strlen(buf);
            if (len + 5 < bufSize) {
                snprintf(buf + len, bufSize - len, " л/ч");
            }
            break;
        }
        case 4:
            snprintf(buf, bufSize, "%d°С", conf.target_temperature_c);
            break;
        case 5:
            snprintf(buf, bufSize, "%d°С", conf.cooling_target_c);
            break;
        case 6:
            snprintf(buf, bufSize, "%d°С", conf.temperature_hysteresis_c);
            break;
        case 7:
            snprintf(buf, bufSize, "%luс", static_cast<unsigned long>(conf.ignitor_timeout_s));
            break;
        case 8:
            snprintf(buf, bufSize, "%d%%", pwmToPercent(conf.motor_PWM_1));
            break;
        case 9:
            snprintf(buf, bufSize, "%d%%", pwmToPercent(conf.motor_PWM_2));
            break;
        case 10:
            snprintf(buf, bufSize, "%d%%", pwmToPercent(conf.motor_PWM_3));
            break;
        case 11:
            snprintf(buf, bufSize, "%d%%", pwmToPercent(conf.motor_PWM_4));
            break;
        case 12:
            snprintf(buf, bufSize, "%u мс", conf.pump_pulse_1);
            break;
        case 13:
            snprintf(buf, bufSize, "%u мс", conf.pump_pulse_2);
            break;
        case 14:
            snprintf(buf, bufSize, "%u мс", conf.pump_pulse_3);
            break;
        case 15:
            snprintf(buf, bufSize, "%u мс", conf.pump_pulse_4);
            break;
        default:
            buf[0] = '\0';
            break;
    }
}

void MainMenuScreen::drawHeader(TFT_eSPI& tft) {
    tft.loadFont(smooth_font::small);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CLR_VALUE, CLR_BG);
    tft.drawString(F("Настройки"), 8, 0);
    tft.unloadFont();

    IconDraw::drawIcon(tft,
                       170 + icon::small::BOX_W / 2,
                       icon::small::BOX_H / 2,
                       icon::small::font,
                       icon::small::SCRUBBER,
                       CLR_ACCENT_GREEN,
                       CLR_BG);

    tft.loadFont(smooth_font::small);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CLR_LABEL, CLR_BG);
    tft.drawString(F("-Сохр."), 170 + icon::small::BOX_W + 2, 2);
    tft.unloadFont();

    IconDraw::drawIcon(tft,
                       250 + icon::small::BOX_W / 2,
                       icon::small::BOX_H / 2,
                       icon::small::font,
                       icon::small::POWER_OFF,
                       CLR_IGN_ICON,
                       CLR_BG);
                       
    tft.loadFont(smooth_font::small);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CLR_LABEL, CLR_BG);
    tft.drawString(F("-Отмена"), 250 + icon::small::BOX_W + 2, 2);
    tft.unloadFont();

    tft.drawFastHLine(0, HEADER_H - 1, 320, CLR_VALUE);
    tft.setTextFont(1);
}

void MainMenuScreen::drawPageIndicator(TFT_eSPI& tft) {
    static constexpr int16_t kTotalH =
        PAGE_COUNT * PAGE_IND_BAR_H + (PAGE_COUNT - 1) * PAGE_IND_GAP;
    static constexpr int16_t kTopY = (240 + HEADER_H - kTotalH) / 2;

    for (uint8_t p = 0; p < PAGE_COUNT; ++p) {
        const int16_t y = kTopY + static_cast<int16_t>(p) *
                                     (PAGE_IND_BAR_H + PAGE_IND_GAP);
        const uint16_t color = (p == m_currentPage) ? CLR_ACCENT_BLUE : CLR_LABEL;
        tft.fillRect(PAGE_IND_X, y, PAGE_IND_BAR_W, PAGE_IND_BAR_H, color);
    }
}

void MainMenuScreen::drawPage(TFT_eSPI& tft, const ConfigManager& cfg) {
    drawPageItems(tft, m_currentPage, cfg);
    drawPageIndicator(tft);
}

void MainMenuScreen::drawPageItems(TFT_eSPI& tft, uint8_t page,
                                    const ConfigManager& cfg) {
    const uint8_t base = static_cast<uint8_t>(page * 8);
    for (uint8_t i = 0; i < 8; ++i) {
        drawItem(tft, static_cast<uint8_t>(base + i), cfg);
    }
}

void MainMenuScreen::drawItem(TFT_eSPI& tft, uint8_t index, const ConfigManager& cfg) {
    if (itemPage(index) != m_currentPage) {
        return;
    }

    int16_t screenX = 0;
    int16_t screenY = 0;
    getItemScreenPos(index, screenX, screenY);

    const bool selected       = (index == m_selectedIndex);
    const uint16_t bg         = selected ? CLR_SELECTION : CLR_BG;
    const uint16_t labelColor = selected ? CLR_ACCENT_GREEN : CLR_LABEL;

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::XL);
    if (spr == nullptr) {
        const SpriteScreenRect rect = {screenX, screenY, SpriteSlot::XL};
        SpritePool::drawOomMarker(tft, rect);
        return;
    }

    spr->fillSprite(CLR_BG);
    if(selected){
        spr->fillSmoothRoundRect(0, 0, SpritePool::XL_W, SpritePool::XL_H, 5, CLR_SELECTION, CLR_BG);
    }

    spr->loadFont(smooth_font::small);
    spr->setTextDatum(TL_DATUM);
    spr->setTextColor(labelColor, bg);
    spr->drawString(LABELS[index], ITEM_PAD_X, ITEM_LABEL_Y);
    spr->unloadFont();

    char valBuf[24] = "";
    formatItemValue(index, cfg.getConfig(), valBuf, sizeof(valBuf));

    spr->loadFont(smooth_font::def);
    spr->setTextDatum(TL_DATUM);
    spr->setTextColor(CLR_VALUE, bg);
    spr->drawString(valBuf, ITEM_PAD_X, ITEM_VALUE_Y);
    spr->unloadFont();

    spr->pushSprite(screenX, screenY);
    SpritePool::release(SpriteSlot::XL);
    tft.setTextFont(1);
}

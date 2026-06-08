#include "display/screens/CalibrationScreen.h"

#include <TFT_eSPI.h>
#include <cstdint>
#include <stdio.h>

#include "display/IconDraw.h"
#include "display/PaletteRGB565.h"
#include "display/SpritePool.h"
#include "display/assets/fonts.h"
#include "display/assets/icons.h"
#include "system/ConfigManager.h"
#include "system/SystemState.h"

namespace {

struct StepMeta {
    const char* label;
    const char* icon;
};

static constexpr int16_t SCREEN_CX = 160;
static constexpr int16_t TITLE_Y   = 9;
static constexpr int16_t CENTER_Y  = 100;
static constexpr int16_t COLUMN_Y = 187;

static constexpr int16_t CENTER_TEXT_GAP        = 6;
static constexpr int16_t CENTER_WIDGET_X_OFFSET = 15;
static constexpr int16_t BOTTOM_GAP             = 5;
static constexpr int16_t VALUE_Y_IN_SPRITE      = 18;

static const StepMeta kSteps[CalibrationScreen::STEP_COUNT] = {
    { "Вкл",   icon::large::POWER },
    { "Вверх", icon::large::UP },
    { "Насос", icon::large::DROPLET },
    { "Свеча", icon::large::FIRE },
    { "Вниз",  icon::large::DOWN },
};

static const SpriteScreenRect kCenterScreen = {
    static_cast<int16_t>(SCREEN_CX - SpritePool::L_W / 2 + CENTER_WIDGET_X_OFFSET),
    static_cast<int16_t>(CENTER_Y - SpritePool::L_H / 2),
    SpriteSlot::L,
};

int16_t bottomColumnScreenX(uint8_t index) {
    return static_cast<int16_t>(index * (SpritePool::S_W + BOTTOM_GAP));
}

}  // namespace

void CalibrationScreen::drawLabelValue(TFT_eSprite& spr,
                                       const int16_t x,
                                       const int16_t y,
                                       const char* label,
                                       const char* value,
                                       const int16_t valueY,
                                       uint16_t valColor) {
    spr.loadFont(smooth_font::small);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(CLR_LABEL, CLR_BG);
    spr.drawString(label, x, y);
    spr.unloadFont();

    spr.loadFont(smooth_font::def);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(valColor, CLR_BG);
    spr.drawString(value, x, static_cast<int16_t>(y + valueY));
    spr.unloadFont();
}

void CalibrationScreen::loadFromConfig(const ConfigManager& cfg) {
    const auto& c = cfg.getConfig();
    m_draft[0] = c.btnPower;
    m_draft[1] = c.btnUp;
    m_draft[2] = c.btnPump;
    m_draft[3] = c.btnIgnition;
    m_draft[4] = c.btnDown;
    m_activeStep = 0;
    for (uint8_t i = 0; i < STEP_COUNT; ++i) {
        m_lastDrawn[i] = 0xFFFFu;
    }
    m_fullRedrawNeeded = true;
}

void CalibrationScreen::onEnter(TFT_eSPI& tft,
                                const SystemState& /*state*/,
                                const ConfigManager& cfg) {
    loadFromConfig(cfg);
    drawAll(tft);
}

void CalibrationScreen::onExit() {
    m_fullRedrawNeeded = true;
}

void CalibrationScreen::tick(TFT_eSPI& tft,
                              const SystemState& state,
                              const ConfigManager& /*cfg*/) {
    if (m_fullRedrawNeeded) {
        drawAll(tft);
        return;
    }

    const auto adc = state.getKeyboardAdcState();
    if (adc.pressed && m_draft[m_activeStep] != adc.stable_adc) {
        m_draft[m_activeStep] = adc.stable_adc;
        drawCenterWidget(tft);
        drawBottomColumn(tft, m_activeStep);
        m_lastDrawn[m_activeStep] = m_draft[m_activeStep];
    }
}

void CalibrationScreen::advanceStep() {
    m_activeStep = static_cast<uint8_t>((m_activeStep + 1) % STEP_COUNT);
}

bool CalibrationScreen::saveToConfig(ConfigManager& cfg) const {
    ConfigManager::Config conf = cfg.getConfig();
    conf.btnPower    = m_draft[0];
    conf.btnUp       = m_draft[1];
    conf.btnPump     = m_draft[2];
    conf.btnIgnition = m_draft[3];
    conf.btnDown     = m_draft[4];
    cfg.setConfig(conf);
    return cfg.save();
}

void CalibrationScreen::refresh(TFT_eSPI& tft) {
    drawCenterWidget(tft);
}

void CalibrationScreen::drawAll(TFT_eSPI& tft) {
    tft.fillScreen(CLR_BG);
    drawTitle(tft);
    drawCenterWidget(tft);
    drawBottomRow(tft);
    m_fullRedrawNeeded = false;
}

void CalibrationScreen::drawTitle(TFT_eSPI& tft) {
    tft.loadFont(smooth_font::small);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(CLR_LABEL, CLR_BG);
    tft.drawString("Калибровка клавиатуры",
                   SCREEN_CX, TITLE_Y);
    tft.unloadFont();
}

void CalibrationScreen::drawCenterWidget(TFT_eSPI& tft) {
    const StepMeta& step = kSteps[m_activeStep];

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::L);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, kCenterScreen);
        return;
    }

    char buf[8];
    snprintf(buf, sizeof(buf), "%u", m_draft[m_activeStep]);

    const int16_t textX = static_cast<int16_t>(icon::large::BOX_W + CENTER_TEXT_GAP);
    const int16_t textY = static_cast<int16_t>((SpritePool::L_H - 32) / 2);
    static const int8_t yOffset = 2;

    spr->fillSprite(CLR_BG);
    IconDraw::drawIcon(*spr,
                       icon::large::BOX_W / 2,
                       SpritePool::L_H / 2 + yOffset,
                       icon::large::font,
                       step.icon,
                       CLR_ACCENT_GREEN,
                       CLR_BG);
    drawLabelValue(*spr, textX, textY-yOffset, step.label, buf, VALUE_Y_IN_SPRITE, CLR_VALUE);

    spr->pushSprite(kCenterScreen.x, kCenterScreen.y);
    SpritePool::release(SpriteSlot::L);

    m_lastDrawn[m_activeStep] = m_draft[m_activeStep];
}

void CalibrationScreen::drawBottomColumn(TFT_eSPI& tft, uint8_t index) {
    const SpriteScreenRect screen = {
        bottomColumnScreenX(index),
        COLUMN_Y,
        SpriteSlot::S,
    };

    TFT_eSprite* spr = SpritePool::acquire(SpriteSlot::S);
    if (spr == nullptr) {
        SpritePool::drawOomMarker(tft, screen);
        return;
    }

    char buf[8];
    snprintf(buf, sizeof(buf), "%u", m_draft[index]);

    spr->fillSprite(CLR_BG);
    drawLabelValue(*spr, 0, 0, kSteps[index].label, buf, VALUE_Y_IN_SPRITE, CLR_VALUE);
    spr->pushSprite(screen.x, screen.y);
    SpritePool::release(SpriteSlot::S);
}

void CalibrationScreen::drawBottomRow(TFT_eSPI& tft) {
    for (uint8_t i = 0; i < STEP_COUNT; ++i) {
        drawBottomColumn(tft, i);
        m_lastDrawn[i] = m_draft[i];
    }
}

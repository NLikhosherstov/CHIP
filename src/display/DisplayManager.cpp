#include "display/DisplayManager.h"

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "display/BaseScreen.h"
#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"
#include "display/screens/CalibrationScreen.h"
#include "display/screens/HomeScreen.h"
#include "display/screens/MainMenuScreen.h"
#include "display/SpritePool.h"
#include "pins.h"
#include "system/SerialProfile.h"
#include "system/SystemState.h"
#include "assets/fonts.h"

namespace {

bool quickMenuConfigChanged(const ConfigManager::Config& snapshot,
                            const ConfigManager::Config& live) {
    return snapshot.target_temperature_c != live.target_temperature_c ||
           snapshot.fuel_correction != live.fuel_correction;
}

}  // namespace

DisplayManager::DisplayManager()
    : m_tft(new TFT_eSPI())
    , m_active(nullptr)
    , m_currentId(ScreenId::HOME)
    , m_homeScreen(new HomeScreen())
    , m_mainMenuScreen(new MainMenuScreen())
    , m_calibrationScreen(new CalibrationScreen())
    , m_mainMenuEditActive(false)
    , m_quickMenuActive(false) {}

DisplayManager::~DisplayManager() {
    delete m_homeScreen;
    delete m_mainMenuScreen;
    delete m_calibrationScreen;
    delete m_tft;
}

bool DisplayManager::needsKeyboardCalibration(const ConfigManager& cfg) {
    const auto& c = cfg.getConfig();
    return c.btnPower == 0 && c.btnUp == 0 && c.btnIgnition == 0 &&
           c.btnPump == 0 && c.btnDown == 0;
}

void DisplayManager::initTft() {
    pinMode(pin::DISPLAY_LIGHT, OUTPUT);
    backlightOff();

    m_tft->setRotation(3);
    m_tft->init();
}

void DisplayManager::beginMaintenance(const __FlashStringHelper* message) {
    initTft();
    m_tft->fillScreen(CLR_BG);
    m_tft->loadFont(smooth_font::small);
    m_tft->setTextDatum(MC_DATUM);
    m_tft->setTextColor(CLR_VALUE, CLR_BG);
    m_tft->drawString(message, m_tft->width() / 2, m_tft->height() / 2);
    m_tft->unloadFont();
    m_tft->setTextFont(1);
    backlightOn();
}

void DisplayManager::endMaintenance() {
    m_tft->fillScreen(CLR_BG);
}

void DisplayManager::begin(const SystemState& state, const ConfigManager& cfg) {
    initTft();
    m_tft->fillScreen(CLR_BG);

    backlightOn();

    if (needsKeyboardCalibration(cfg)) {
        switchTo(ScreenId::CALIBRATION, state, cfg);
    } else {
        m_active = m_homeScreen;
        m_currentId = ScreenId::HOME;
        m_active->onEnter(*m_tft, state, cfg);
    }
}

void DisplayManager::tick(SystemState& state, ConfigManager& cfg) {
    const auto req = state.getRequests();
    if (req.enter_calibration) {
        state.clearRequestEnterCalibration();
        openCalibration(state, cfg);
    }

    if (m_active) {
        m_active->tick(*m_tft, state, cfg);
    }

    if (isQuickMenuVisible() && m_homeScreen->isQuickMenuTimedOut()) {
        closeQuickMenu(state, cfg);
    }
}

void DisplayManager::switchTo(ScreenId id, const SystemState& state, const ConfigManager& cfg) {
    if (id == m_currentId) {
        return;
    }

    if (m_active) {
        m_active->onExit();
    }

    // При входе в настройки — снимок для «выход без сохранения» (Power).
    if (id == ScreenId::MAIN_MENU) {
        m_mainMenuSnapshot = cfg.getConfig();
        m_mainMenuEdit = m_mainMenuSnapshot;
        m_mainMenuEditActive = true;
    } else {
        m_mainMenuEditActive = false;
    }

    switch (id) {
        case ScreenId::HOME:
            m_active = m_homeScreen;
            break;
        case ScreenId::MAIN_MENU:
            m_active = m_mainMenuScreen;
            break;
        case ScreenId::CALIBRATION:
            m_active = m_calibrationScreen;
            SpritePool::init(*m_tft);
            break;
        default:
            m_active = m_homeScreen;
            break;
    }

    m_currentId = id;

    if (m_active) {
        m_active->onEnter(*m_tft, state, cfg);
    }
}

DisplayManager::ScreenId DisplayManager::currentScreen() const {
    return m_currentId;
}

void DisplayManager::backlightOn() {
    digitalWrite(pin::DISPLAY_LIGHT, HIGH);
}

void DisplayManager::backlightOff() {
    digitalWrite(pin::DISPLAY_LIGHT, LOW);
}

bool DisplayManager::isQuickMenuVisible() const {
    return (m_currentId == ScreenId::HOME) && m_homeScreen->isQuickMenuVisible();
}

bool DisplayManager::isCalibrationActive() const {
    return m_currentId == ScreenId::CALIBRATION;
}

bool DisplayManager::isMenuActive() const {
    return isQuickMenuVisible() || (m_currentId == ScreenId::MAIN_MENU);
}

bool DisplayManager::isQuickMenuModeItemVisible(const SystemState& state) const {
    return state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL;
}

void DisplayManager::openQuickMenu(uint8_t default_item, const SystemState& state, const ConfigManager& cfg) {
    if (m_currentId != ScreenId::HOME) {
        return;
    }
    // В AUTO пункт «Режим» (индекс 2) скрыт — сдвигаем default при открытии с «Вниз».
    uint8_t storage = default_item;
    if (!isQuickMenuModeItemVisible(state) && default_item >= 2) {
        storage = static_cast<uint8_t>(default_item + 1);
    }
    if (storage >= HomeScreen::QM_ITEM_COUNT) {
        storage = HomeScreen::QM_ITEM_COUNT - 1;
    }
    m_quickMenuSnapshot = cfg.getConfig();
    m_quickMenuActive = true;
    m_homeScreen->showQuickMenu(*m_tft, state, cfg, storage);
}

void DisplayManager::closeQuickMenu(const SystemState& state, ConfigManager& cfg) {
    if (m_currentId != ScreenId::HOME) {
        return;
    }

    if (m_quickMenuActive) {
        if (quickMenuConfigChanged(m_quickMenuSnapshot, cfg.getConfig())) {
            cfg.save();
        }
        m_quickMenuActive = false;
    }
    m_homeScreen->hideQuickMenu(*m_tft, state, cfg);
}

void DisplayManager::openMainMenu(const SystemState& state, ConfigManager& cfg) {
    closeQuickMenu(state, cfg);
    switchTo(ScreenId::MAIN_MENU, state, cfg);
}

void DisplayManager::openCalibration(const SystemState& state, ConfigManager& cfg) {
    closeQuickMenu(state, cfg);
    switchTo(ScreenId::CALIBRATION, state, cfg);
}

void DisplayManager::adjustSelectedItem(int8_t delta, SystemState& state, ConfigManager& cfg) {
    if (isQuickMenuVisible()) {
        m_homeScreen->quickMenuAdjust(*m_tft, delta, state, cfg);
        return;
    }
    if (m_currentId == ScreenId::MAIN_MENU && m_mainMenuEditActive) {
        ConfigManager edit_cfg;
        edit_cfg.setConfig(m_mainMenuEdit);
        const uint8_t idx = m_mainMenuScreen->selectedIndex();
        m_mainMenuScreen->adjustValue(delta, edit_cfg);
        m_mainMenuEdit = edit_cfg.getConfig();
        m_mainMenuScreen->refreshItem(*m_tft, idx, edit_cfg);
    }
}

// Клик энкодера в быстром меню: числовые поля +1, «Режим» → запрос AUTO, «Меню» → главное меню.
void DisplayManager::applyClickToSelectedItem(SystemState& state, ConfigManager& cfg) {
    if (!isQuickMenuVisible()) {
        return;
    }

    const uint8_t item = m_homeScreen->quickMenuSelectedItem();
    switch (item) {
        case 0:
        case 1:
            adjustSelectedItem(1, state, cfg);
            break;
        case 2:
            if (state.getAutomationState() == SystemState::AutomationState::STATE_MANUAL) {
                state.postEnterAutoRequest();
            }
            break;
        case 3:
            openMainMenu(state, cfg);
            break;
        default:
            break;
    }
}

void DisplayManager::exitMainMenuSave(ConfigManager& cfg, const SystemState& state) {
    if (m_mainMenuEditActive) {
        cfg.setConfig(m_mainMenuEdit);
        cfg.save();
    }
    switchTo(ScreenId::HOME, state, cfg);
}

void DisplayManager::exitMainMenuDiscard(ConfigManager& cfg, const SystemState& state) {
    if (m_mainMenuEditActive) {
        cfg.setConfig(m_mainMenuSnapshot);
    }
    switchTo(ScreenId::HOME, state, cfg);
}

void DisplayManager::event(Event& e) {
    if (isCalibrationActive()) {
        handleUiInput_Calibration(e.type, e.payload, e.state, e.config);
        e.accept = true;
        return;
    }

    if (m_currentId == ScreenId::MAIN_MENU) {
        if (e.type == EventType::LongClick) {
            openCalibration(e.state, e.config);
        } else {
            handleUiInput_MainMenu(e.type, e.payload, e.state, e.config);
        }
        e.accept = true;
        return;
    }

    if (isQuickMenuVisible()) {
        handleUiInput_QuickMenu(e.type, e.payload, e.state, e.config);
        e.accept = true;
        return;
    }

    if (e.type == EventType::Up || e.type == EventType::Down) {
        handleUiInput_Home(e.type, e.payload, e.state, e.config);
        e.accept = true;
        return;
    }

    if (e.type == EventType::Click) {
        openMainMenu(e.state, e.config);
        e.accept = true;
    }
}

void DisplayManager::handleUiInput_Home(EventType action, int16_t payload, SystemState& state, ConfigManager& cfg) {
    (void)payload;

    if (isQuickMenuVisible()) {
        handleUiInput_QuickMenu(action, payload, state, cfg);
        return;
    }

    switch (action) {
        case EventType::Up:
            openQuickMenu(0, state, cfg);
            break;
        case EventType::Down:
            openQuickMenu(3, state, cfg);
            break;
        default:
            break;
    }
}

// Быстрое меню: Up/Down — навигация; Left/Right — выход; Delta — значение; Click — действие по пункту.
void DisplayManager::handleUiInput_QuickMenu(EventType action, int16_t payload, SystemState& state, ConfigManager& cfg) {
    switch (action) {
        case EventType::Up:
            m_homeScreen->quickMenuSelectPrev(*m_tft, state, cfg);
            break;
        case EventType::Down:
            m_homeScreen->quickMenuSelectNext(*m_tft, state, cfg);
            break;
        case EventType::Left:
        case EventType::Right:
            closeQuickMenu(state, cfg);
            break;
        case EventType::Delta:
            adjustSelectedItem(static_cast<int8_t>(payload), state, cfg);
            break;
        case EventType::Click:
            applyClickToSelectedItem(state, cfg);
            break;
        default:
            break;
    }
}

// Главное меню: Up/Down/Left/Right — навигация; Click — сохранить; Power — отменить; Delta — правка.
void DisplayManager::handleUiInput_MainMenu(EventType action, int16_t payload, SystemState& state, ConfigManager& cfg) {
    ConfigManager view;
    if (m_mainMenuEditActive) {
        view.setConfig(m_mainMenuEdit);
    }

    switch (action) {
        case EventType::Up:
        case EventType::Down:
        case EventType::Left:
        case EventType::Right: {
            const uint8_t oldIndex = m_mainMenuScreen->selectedIndex();
            if (!m_mainMenuScreen->navigate(action)) {
                break;
            }
            const uint8_t newIndex = m_mainMenuScreen->selectedIndex();
            if (m_mainMenuEditActive) {
                m_mainMenuScreen->refreshSelection(*m_tft, oldIndex, newIndex, view);
            } else {
                m_mainMenuScreen->refreshSelection(*m_tft, oldIndex, newIndex, cfg);
            }
            break;
        }
        case EventType::Delta:
            adjustSelectedItem(static_cast<int8_t>(payload), state, cfg);
            break;
        case EventType::Click:
            exitMainMenuSave(cfg, state);
            break;
        case EventType::Power:
            exitMainMenuDiscard(cfg, state);
            break;
        default:
            break;
    }
}

void DisplayManager::handleUiInput_Calibration(EventType action,
                                               int16_t payload,
                                               SystemState& state,
                                               ConfigManager& cfg) {
    (void)payload;

    switch (action) {
        case EventType::Click:
            m_calibrationScreen->advanceStep();
            m_calibrationScreen->refresh(*m_tft);
            break;
        case EventType::LongClick:
            m_calibrationScreen->saveToConfig(cfg);
            state.postReloadKeyboardCalRequest();
            switchTo(ScreenId::HOME, state, cfg);
            break;
        default:
            break;
    }
}

#include "display/DisplayManager.h"

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "display/BaseScreen.h"
#include "display/PaletteRGB565.h"
#include "display/screens/HomeScreen.h"
#include "display/screens/MainMenuScreen.h"
#include "pins.h"
#include "system/SystemState.h"

DisplayManager::DisplayManager()
    : m_tft(new TFT_eSPI())
    , m_active(nullptr)
    , m_currentId(ScreenId::HOME)
    , m_homeScreen(new HomeScreen())
    , m_mainMenuScreen(new MainMenuScreen())
    , m_mainMenuEditActive(false) {}

DisplayManager::~DisplayManager() {
    delete m_homeScreen;
    delete m_mainMenuScreen;
    delete m_tft;
}

void DisplayManager::begin(const SystemState& state, const ConfigManager& cfg) {
    pinMode(pin::DISPLAY_LIGHT, OUTPUT);
    backlightOff();

    m_tft->setRotation(3);
    m_tft->init();
    m_tft->fillScreen(CLR_BG);

    backlightOn();

    m_active = m_homeScreen;
    m_currentId = ScreenId::HOME;
    m_active->onEnter(*m_tft, state, cfg);
}

void DisplayManager::tick(const SystemState& state, const ConfigManager& cfg) {
    if (m_active) {
        m_active->tick(*m_tft, state, cfg);
    }
}

void DisplayManager::switchTo(ScreenId id, const SystemState& state, const ConfigManager& cfg) {
    if (id == m_currentId) {
        return;
    }

    if (m_active) {
        m_active->onExit();
    }

    // При входе в настройки — снимок для «выход без сохранения» (Right).
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
    m_homeScreen->showQuickMenu(*m_tft, state, cfg, storage);
}

void DisplayManager::closeQuickMenu(const SystemState& state, const ConfigManager& cfg) {
    if (m_currentId == ScreenId::HOME) {
        m_homeScreen->hideQuickMenu(*m_tft, state, cfg);
    }
}

void DisplayManager::openMainMenu(const SystemState& state, const ConfigManager& cfg) {
    closeQuickMenu(state, cfg);
    switchTo(ScreenId::MAIN_MENU, state, cfg);
}

void DisplayManager::adjustSelectedItem(int8_t delta, SystemState& state, ConfigManager& cfg) {
    if (isQuickMenuVisible()) {
        m_homeScreen->quickMenuAdjust(*m_tft, delta, state, cfg);
        return;
    }
    if (m_currentId == ScreenId::MAIN_MENU && m_mainMenuEditActive) {
        ConfigManager edit_cfg;
        edit_cfg.setConfig(m_mainMenuEdit);
        m_mainMenuScreen->adjustValue(delta, edit_cfg);
        m_mainMenuEdit = edit_cfg.getConfig();
        m_mainMenuScreen->refresh(*m_tft, edit_cfg);
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

void DisplayManager::handleUiInput_Home(UiInput action, int16_t payload, SystemState& state, ConfigManager& cfg) {
    (void)payload;

    if (isQuickMenuVisible()) {
        handleUiInput_QuickMenu(action, payload, state, cfg);
        return;
    }

    switch (action) {
        case UiInput::Up:
            openQuickMenu(0, state, cfg);
            break;
        case UiInput::Down:
            openQuickMenu(3, state, cfg);
            break;
        default:
            break;
    }
}

// Быстрое меню: Up/Down — навигация; Left/Right — выход; Delta — значение; Click — действие по пункту.
void DisplayManager::handleUiInput_QuickMenu(UiInput action, int16_t payload, SystemState& state, ConfigManager& cfg) {
    switch (action) {
        case UiInput::Up:
            m_homeScreen->quickMenuSelectPrev(*m_tft, state, cfg);
            break;
        case UiInput::Down:
            m_homeScreen->quickMenuSelectNext(*m_tft, state, cfg);
            break;
        case UiInput::Left:
        case UiInput::Right:
            closeQuickMenu(state, cfg);
            break;
        case UiInput::Delta:
            adjustSelectedItem(static_cast<int8_t>(payload), state, cfg);
            break;
        case UiInput::Click:
            applyClickToSelectedItem(state, cfg);
            break;
        case UiInput::OpenMainMenu:
            openMainMenu(state, cfg);
            break;
        default:
            break;
    }
}

// Главное меню: Left/Click — сохранить и выйти; Right — отменить; Delta — правка параметра.
void DisplayManager::handleUiInput_MainMenu(UiInput action, int16_t payload, SystemState& state, ConfigManager& cfg) {
    switch (action) {
        case UiInput::Up:
            m_mainMenuScreen->selectPrev();
            if (m_mainMenuEditActive) {
                ConfigManager view;
                view.setConfig(m_mainMenuEdit);
                m_mainMenuScreen->refresh(*m_tft, view);
            } else {
                m_mainMenuScreen->refresh(*m_tft, cfg);
            }
            break;
        case UiInput::Down:
            m_mainMenuScreen->selectNext();
            if (m_mainMenuEditActive) {
                ConfigManager view;
                view.setConfig(m_mainMenuEdit);
                m_mainMenuScreen->refresh(*m_tft, view);
            } else {
                m_mainMenuScreen->refresh(*m_tft, cfg);
            }
            break;
        case UiInput::Delta:
            adjustSelectedItem(static_cast<int8_t>(payload), state, cfg);
            break;
        case UiInput::Left:
        case UiInput::Click:
            exitMainMenuSave(cfg, state);
            break;
        case UiInput::Right:
            exitMainMenuDiscard(cfg, state);
            break;
        default:
            break;
    }
}

void DisplayManager::handleUiInput(UiInput action,
                                   int16_t payload,
                                   SystemState& state,
                                   ConfigManager& cfg) {
    if (action == UiInput::OpenMainMenu) {
        openMainMenu(state, cfg);
        return;
    }

    if (m_currentId == ScreenId::MAIN_MENU) {
        handleUiInput_MainMenu(action, payload, state, cfg);
        return;
    }

    handleUiInput_Home(action, payload, state, cfg);
}

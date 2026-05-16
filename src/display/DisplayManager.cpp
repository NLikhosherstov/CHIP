#include "display/DisplayManager.h"

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "display/BaseScreen.h"
#include "display/screens/HomeScreen.h"
#include "display/screens/MainMenuScreen.h"
#include "pins.h"

DisplayManager::DisplayManager()
    : m_tft(new TFT_eSPI())
    , m_active(nullptr)
    , m_currentId(ScreenId::HOME)
    , m_homeScreen(new HomeScreen())
    , m_mainMenuScreen(new MainMenuScreen())
{}

DisplayManager::~DisplayManager(){
    delete m_homeScreen;
    delete m_mainMenuScreen;
    delete m_tft;
}

void DisplayManager::begin(const SystemState& state, const ConfigManager& cfg){
    pinMode(pin::DISPLAY_LIGHT, OUTPUT);
    backlightOff();

    m_tft->setRotation(3);  // Landscape, 0,0 — левый верхний угол
    m_tft->init();
    m_tft->fillScreen(0x0042); // Фон #000b16 в RGB565

    backlightOn();

    // Активируем главный экран
    m_active = m_homeScreen;
    m_currentId = ScreenId::HOME;
    m_active->onEnter(*m_tft, state, cfg);
}

void DisplayManager::tick(const SystemState& state, const ConfigManager& cfg){
    if (m_active) {
        m_active->tick(*m_tft, state, cfg);
    }
}

void DisplayManager::switchTo(ScreenId id, const SystemState& state, const ConfigManager& cfg){
    if(id == m_currentId) return;

    if(m_active)
        m_active->onExit();

    switch(id){
        case ScreenId::HOME:      m_active = m_homeScreen;     break;
        case ScreenId::MAIN_MENU: m_active = m_mainMenuScreen; break;
        default:                  m_active = m_homeScreen;     break;
    }

    m_currentId = id;

    if(m_active)
        m_active->onEnter(*m_tft, state, cfg);
}

DisplayManager::ScreenId DisplayManager::currentScreen() const{
    return m_currentId;
}

void DisplayManager::backlightOn(){
    digitalWrite(pin::DISPLAY_LIGHT, HIGH);
}

void DisplayManager::backlightOff(){
    digitalWrite(pin::DISPLAY_LIGHT, LOW);
}

// Делегаты Быстрого меню

void DisplayManager::showQuickMenu(const SystemState& state, const ConfigManager& cfg){
    if (m_currentId == ScreenId::HOME) 
        m_homeScreen->showQuickMenu(*m_tft, state, cfg);
}

void DisplayManager::hideQuickMenu(const SystemState& state, const ConfigManager& cfg){
    if(m_currentId == ScreenId::HOME)
        m_homeScreen->hideQuickMenu(*m_tft, state, cfg);
}

void DisplayManager::quickMenuSelectNext(const SystemState& state, const ConfigManager& cfg){
    if(m_currentId == ScreenId::HOME)
        m_homeScreen->quickMenuSelectNext(*m_tft, state, cfg);
}

void DisplayManager::quickMenuSelectPrev(const SystemState& state, const ConfigManager& cfg){
    if(m_currentId == ScreenId::HOME)
        m_homeScreen->quickMenuSelectPrev(*m_tft, state, cfg);
}

void DisplayManager::quickMenuAdjust(int8_t delta, SystemState& state, ConfigManager& cfg){
    if(m_currentId == ScreenId::HOME)
        m_homeScreen->quickMenuAdjust(*m_tft, delta, state, cfg);
}

bool DisplayManager::isQuickMenuVisible() const {
    return (m_currentId == ScreenId::HOME) && m_homeScreen->isQuickMenuVisible();
}

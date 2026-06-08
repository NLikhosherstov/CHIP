#pragma once
#include <stdint.h>

#include "display/UiInput.h"
#include "system/ConfigManager.h"

class TFT_eSPI;
class SystemState;
class BaseScreen;
class HomeScreen;
class MainMenuScreen;
class CalibrationScreen;

// Главный контроллер дисплея и интерпретатор семантического ввода (UiInput).
class DisplayManager {
public:
    enum class ScreenId : uint8_t {
        HOME        = 0,
        MAIN_MENU   = 1,
        CALIBRATION = 2,
    };

    DisplayManager();
    ~DisplayManager();

    void begin(const SystemState& state, const ConfigManager& cfg);
    void tick(SystemState& state, const ConfigManager& cfg);

    void switchTo(ScreenId id, const SystemState& state, const ConfigManager& cfg);

    ScreenId currentScreen() const;

    void backlightOn();
    void backlightOff();

    static bool needsKeyboardCalibration(const ConfigManager& cfg);

    // Быстрое меню или экран настроек — весь ввод через handleUiInput.
    bool isMenuActive() const;
    bool isCalibrationActive() const;
    bool isQuickMenuVisible() const;

    void handleUiInput(UiInput action,
                       int16_t payload,
                       SystemState& state,
                       ConfigManager& cfg);

private:
    void handleUiInput_Home(UiInput action, int16_t payload, SystemState& state, ConfigManager& cfg);
    void handleUiInput_QuickMenu(UiInput action, int16_t payload, SystemState& state, ConfigManager& cfg);
    void handleUiInput_MainMenu(UiInput action, int16_t payload, SystemState& state, ConfigManager& cfg);
    void handleUiInput_Calibration(UiInput action, int16_t payload, SystemState& state, ConfigManager& cfg);

    void openQuickMenu(uint8_t default_item, const SystemState& state, const ConfigManager& cfg);
    void closeQuickMenu(const SystemState& state, const ConfigManager& cfg);
    void openMainMenu(const SystemState& state, const ConfigManager& cfg);
    void openCalibration(const SystemState& state, const ConfigManager& cfg);

    void adjustSelectedItem(int8_t delta, SystemState& state, ConfigManager& cfg);
    void applyClickToSelectedItem(SystemState& state, ConfigManager& cfg);

    void exitMainMenuSave(ConfigManager& cfg, const SystemState& state);
    void exitMainMenuDiscard(ConfigManager& cfg, const SystemState& state);

    bool isQuickMenuModeItemVisible(const SystemState& state) const;

    TFT_eSPI*      m_tft;
    BaseScreen*    m_active;
    ScreenId       m_currentId;

    HomeScreen*         m_homeScreen;
    MainMenuScreen*     m_mainMenuScreen;
    CalibrationScreen*  m_calibrationScreen;

    ConfigManager::Config m_mainMenuEdit{};      // черновик правок в MAIN_MENU
    ConfigManager::Config m_mainMenuSnapshot{};  // снимок при входе в меню
    bool m_mainMenuEditActive;
};

#pragma once
#include <stdint.h>

class TFT_eSPI;
class SystemState;
class ConfigManager;
class BaseScreen;
class HomeScreen;
class MainMenuScreen;

// Главный контроллер дисплея.
// Владеет объектом TFT_eSPI и управляет активным экраном.
// Экранов всего два: HOME (главный) и MAIN_MENU (меню настроек).
// Быстрое меню является модальным виджетом ВНУТРИ HomeScreen, а не отдельным экраном.
class DisplayManager {
public:
    enum class ScreenId : uint8_t {
        HOME      = 0,
        MAIN_MENU = 1,
    };

    DisplayManager();
    ~DisplayManager();

    // Инициализация TFT_eSPI, установка ориентации, полный сброс экрана.
    void begin(const SystemState& state, const ConfigManager& cfg);

    // Вызывается каждую итерацию главного цикла.
    // Делегирует в tick() активного экрана.
    void tick(const SystemState& state, const ConfigManager& cfg);

    // Переключение активного экрана.
    // Вызывает onExit() на текущем и onEnter() на следующем.
    void switchTo(ScreenId id, const SystemState& state, const ConfigManager& cfg);

    ScreenId currentScreen() const;

    // Управление подсветкой
    void backlightOn();
    void backlightOff();

    // Делегаты управления Быстрым меню (HomeScreen)
    // Открывает/закрывает оверлей быстрого меню поверх главного экрана.
    void showQuickMenu(const SystemState& state, const ConfigManager& cfg);
    void hideQuickMenu(const SystemState& state, const ConfigManager& cfg);

    // Навигация по пунктам быстрого меню.
    void quickMenuSelectNext(const SystemState& state, const ConfigManager& cfg);
    void quickMenuSelectPrev(const SystemState& state, const ConfigManager& cfg);

    // Изменение значения выбранного пункта. delta = +1 или -1.
    void quickMenuAdjust(int8_t delta, SystemState& state, ConfigManager& cfg);

    bool isQuickMenuVisible() const;

private:
    TFT_eSPI*      m_tft;
    BaseScreen*    m_active; // Не владеющий указатель на один из экранов
    ScreenId       m_currentId;

    // Экземпляры экранов — статические объекты (без динамической аллокации).
    // В каждый момент активен только один
    HomeScreen*     m_homeScreen;
    MainMenuScreen* m_mainMenuScreen;
};

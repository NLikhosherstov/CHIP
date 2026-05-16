#pragma once

class TFT_eSPI;
class SystemState;
class ConfigManager;

// Абстрактный интерфейс экрана.
// Каждый экран реализует три жизненных метода:
//   onEnter — полная отрисовка при входе на экран
//   onExit  — очистка при уходе с экрана
//   tick    — вызывается каждый цикл: перерисовывает только изменившиеся области
class BaseScreen {
public:
    virtual ~BaseScreen() = default;

    // Полная отрисовка. Вызывается один раз при активации экрана.
    virtual void onEnter(TFT_eSPI& tft,
                         const SystemState& state,
                         const ConfigManager& cfg) = 0;

    // Освобождение ресурсов при деактивации экрана.
    virtual void onExit() = 0;

    // Обновление экрана. Вызывается из главного цикла.
    // Должен перерисовывать только «грязные» (изменившиеся) области,
    // чтобы минимизировать трафик SPI.
    virtual void tick(TFT_eSPI& tft,
                      const SystemState& state,
                      const ConfigManager& cfg) = 0;
};

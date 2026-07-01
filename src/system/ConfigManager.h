#pragma once

#include <stdint.h>

class ConfigManager {
public:
  struct __attribute__((__packed__)) Config {

    // Данные калибровки клавиатуры
    uint16_t btnPower             = 0;
    uint16_t btnUp                = 0;
    uint16_t btnIgnition          = 0;
    uint16_t btnPump              = 0;
    uint16_t btnDown              = 0;

    // Интервал работы насоса соответствующий скорости двигателя в мс
    uint16_t pump_pulse_1         = 328;
    uint16_t pump_pulse_2         = 252;
    uint16_t pump_pulse_3         = 190;
    uint16_t pump_pulse_4         = 153;

    // Производительность насоса в микролитрах(мл*1000) за импульс.
    // Базовая производительность насоса ТН-10 = 6.4ml/100
    uint16_t pumpPerfomance       = 64;

    // Максимальный расход топлива, десятые л/ч (22 = 2.2 л/ч)
    uint16_t max_fuel_flow_lph_x10 = 22;

    // Общая коррекция подачи топлива(изменение интервала импульсов на заданный процент)
    int16_t  fuel_correction      = 0;

    // Максимальное время работы свечи накала между включениями
    uint16_t ignitor_timeout_s    = 60;

    // Ступенчатая регулировка скорости двигателя, permille (1000 = 100%)
    uint16_t motor_PWM_1          = 550;
    uint16_t motor_PWM_2          = 700;
    uint16_t motor_PWM_3          = 850;
    uint16_t motor_PWM_4          = 1000;

    // Температура термостатирования
    uint8_t  target_temperature_c = 25;

    // Температура остановки охлаждения теплообменника
    uint8_t  cooling_target_c         = 45;

    // Гистерезис термостата, °C
    uint8_t  temperature_hysteresis_c = 7;

    // Режим работы со штатным насосом
    bool     embededPump          = false;

  };

  struct __attribute__((__packed__)) PersistentStorage{
    uint8_t id = 0xD1;   // Device ID
    uint8_t version = 3; // Версия структуры настроек
    Config  config;      // Сама структура настроек
    uint8_t crc8 = 0;    // Контрольная сумма
  };

  ConfigManager() = default;

  const Config& getConfig() const;
  void setConfig(const Config& config);

  bool load();
  bool isLoaded() const;

  bool save() const;

  void resetToDefaults();

  //Проверка свободного места под хранилище конфигурации
  bool needsCompaction() const;
  bool compact();

  ConfigManager::PersistentStorage buildPersistentStorage() const;

private:
  static Config sanitize(const Config& config);
  static uint8_t calculateCrc8(const PersistentStorage& storage);
  static bool isValidStorage(const PersistentStorage& storage);

private:
  PersistentStorage m_storage{};

  bool m_isLoaded = false;
};

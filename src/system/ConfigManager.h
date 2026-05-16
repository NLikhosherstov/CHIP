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

    // Общая коррекция подачи топлива(изменение интервала импульсов на заданную величину)
    int16_t  fuel_correction      = 0;    
    
    // Максимальное время работы свечи накала между включениями
    uint16_t ignitor_timeout_s    = 60; 
   
    // Ступенчатая регулировка скорости двигателя ШИМ-255
    uint8_t  motor_PWM_1          = 153;   
    uint8_t  motor_PWM_2          = 180;
    uint8_t  motor_PWM_3          = 220;
    uint8_t  motor_PWM_4          = 254;

    // Температура термостатирования
    uint8_t  target_temperature_c = 25;    

    // Режим работы со штатным насосом 
    bool     embededPump          = false;

  };
  
  struct __attribute__((__packed__)) PersistentStorage{
    uint8_t id = 0xD1;   // Device ID
    uint8_t version = 1; // Версия структуры настроек
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

private:
  static Config sanitize(const Config& config);
  static uint8_t calculateCrc8(const PersistentStorage& storage);
  static bool isValidStorage(const PersistentStorage& storage);


private:
  PersistentStorage m_storage{};

  bool m_isLoaded = false;
};

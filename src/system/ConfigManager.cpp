#include "system/ConfigManager.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <stddef.h>
#include <string.h>

namespace{
  constexpr int EEPROM_STORAGE_ADDRESS = 0;
  constexpr uint8_t STORAGE_ID = 0xD1;
  constexpr uint8_t STORAGE_VERSION = 2;
  constexpr uint8_t CRC8_POLYNOMIAL = 0x07;

  template <typename T>
  T clampValue(T value, T min_value, T max_value) {
    if(value < min_value){
      return min_value;
    }
    if(value > max_value){
      return max_value;
    }
    return value;
  }
}  // namespace

const ConfigManager::Config& ConfigManager::getConfig() const {
  return m_storage.config;
}

void ConfigManager::setConfig(const Config& config) {
  m_storage.config = sanitize(config);
}

bool ConfigManager::load() {
  EEPROM.begin();
  EEPROM.get(EEPROM_STORAGE_ADDRESS, m_storage);

  if(isValidStorage(m_storage)){
    m_storage.config = sanitize(m_storage.config);
    m_isLoaded = true;
    return true;
  }

  // Память пуста или повреждена: восстанавливаем заводские настройки.
  resetToDefaults();
  save();

  PersistentStorage verify_storage{};
  EEPROM.get(EEPROM_STORAGE_ADDRESS, verify_storage);
  if(!isValidStorage(verify_storage)){
    Serial.println(F("CRITICAL: ConfigManager EEPROM/Flash storage verification failed"));
  }

  // Возвращаем false даже при успешном восстановлении дефолтов:
  // пользовательские настройки не были восстановлены.
  return false;
}

bool ConfigManager::save() const {
  EEPROM.begin();

  PersistentStorage temporary_storage = m_storage;
  temporary_storage.id = STORAGE_ID;
  temporary_storage.version = STORAGE_VERSION;
  temporary_storage.crc8 = calculateCrc8(temporary_storage);

  EEPROM.put(EEPROM_STORAGE_ADDRESS, temporary_storage);

  PersistentStorage verify_storage{};
  EEPROM.get(EEPROM_STORAGE_ADDRESS, verify_storage);
  return isValidStorage(verify_storage) &&
         (memcmp(&temporary_storage, &verify_storage, sizeof(PersistentStorage)) == 0);
}

void ConfigManager::resetToDefaults() {
  m_storage.id = STORAGE_ID;
  m_storage.version = STORAGE_VERSION;
  m_storage.config = Config{};
  m_storage.crc8 = calculateCrc8(m_storage);
}

ConfigManager::Config ConfigManager::sanitize(const Config& config) {
  Config sanitized = config;
  sanitized.btnPower             = clampValue<uint16_t>(sanitized.btnPower, 0, 4095);
  sanitized.btnUp                = clampValue<uint16_t>(sanitized.btnUp, 0, 4095);
  sanitized.btnIgnition          = clampValue<uint16_t>(sanitized.btnIgnition, 0, 4095);
  sanitized.btnPump              = clampValue<uint16_t>(sanitized.btnPump, 0, 4095);
  sanitized.btnDown              = clampValue<uint16_t>(sanitized.btnDown, 0, 4095);
  sanitized.pump_pulse_2         = clampValue<uint16_t>(sanitized.pump_pulse_2, 10, 1000);
  sanitized.pump_pulse_1         = clampValue<uint16_t>(sanitized.pump_pulse_1, 10, 1000);
  sanitized.pump_pulse_3         = clampValue<uint16_t>(sanitized.pump_pulse_3, 10, 1000);
  sanitized.pump_pulse_4         = clampValue<uint16_t>(sanitized.pump_pulse_4, 10, 1000);
  sanitized.pumpPerfomance         = clampValue<uint16_t>(sanitized.pumpPerfomance, 10, 1000);
  sanitized.max_fuel_flow_lph_x10  = clampValue<uint16_t>(sanitized.max_fuel_flow_lph_x10, 1, 100);
  sanitized.fuel_correction        = clampValue<int16_t>(sanitized.fuel_correction, -1000, 1000);
  sanitized.ignitor_timeout_s      = clampValue<uint16_t>(sanitized.ignitor_timeout_s, 10, 600);
  sanitized.motor_PWM_1            = clampValue<uint8_t>(sanitized.motor_PWM_1, 0, 255);
  sanitized.motor_PWM_2            = clampValue<uint8_t>(sanitized.motor_PWM_2, 0, 255);
  sanitized.motor_PWM_3            = clampValue<uint8_t>(sanitized.motor_PWM_3, 0, 255);
  sanitized.motor_PWM_4            = clampValue<uint8_t>(sanitized.motor_PWM_4, 0, 255);
  sanitized.target_temperature_c   = clampValue<uint8_t>(sanitized.target_temperature_c, 10, 120);
  sanitized.cooling_target_c       = clampValue<uint8_t>(sanitized.cooling_target_c, 30, 90);
  sanitized.temperature_hysteresis_c =
      clampValue<uint8_t>(sanitized.temperature_hysteresis_c, 1, 20);
  sanitized.embededPump            = sanitized.embededPump;
  return sanitized;
}

uint8_t ConfigManager::calculateCrc8(const PersistentStorage& storage) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&storage);
  const size_t length = offsetof(PersistentStorage, crc8);
  uint8_t crc = 0;

  for(size_t i = 0; i < length; ++i){
    crc ^= bytes[i];
    for(uint8_t bit = 0; bit < 8; ++bit){
      if((crc & 0x80u) != 0u){
        crc = static_cast<uint8_t>((crc << 1u) ^ CRC8_POLYNOMIAL);
      } else {
        crc = static_cast<uint8_t>(crc << 1u);
      }
    }
  }

  return crc;
}

bool ConfigManager::isValidStorage(const PersistentStorage& storage) {
  if(storage.id != STORAGE_ID){
    return false;
  }
  if(storage.version != STORAGE_VERSION){
    return false;
  }
  return calculateCrc8(storage) == storage.crc8;
}

bool ConfigManager::isLoaded() const {
  return m_isLoaded;
}
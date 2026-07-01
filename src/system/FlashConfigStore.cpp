#include "system/FlashConfigStore.h"

#include "system/SerialProfile.h"

#include <stm32f4xx_hal_flash.h>
#include <stm32f4xx_hal_flash_ex.h>
#include <string.h>

#ifndef FLASH_CONFIG_BASE
// Базовый адрес сектора flash для хранения конфигурации (Sector 7 на STM32F411).
#define FLASH_CONFIG_BASE 0x08060000UL
#endif

#ifndef FLASH_CONFIG_SECTOR
// Номер сектора flash, зарезервированного под конфигурацию.
#define FLASH_CONFIG_SECTOR FLASH_SECTOR_7
#endif

namespace {

constexpr uint32_t kSectorBase = FLASH_CONFIG_BASE;
// Размер сектора конфигурации: 128 КБ.
constexpr uint32_t kSectorSize = 128U * 1024U;

// Магия записи: ASCII «HC» (Heater CHIP).
constexpr uint16_t kRecordMagic = 0x4843U;
// Маркер валидной (не стёртой) записи.
constexpr uint8_t kRecordStateValid = 0xA5U;
// Версия формата записи в flash (обёртка вокруг PersistentStorage).
constexpr uint8_t kRecordFormatVersion = 1U;
// Полный размер одной записи в логе: заголовок + payload, выровнен до 64 байт.
constexpr size_t kRecordSize = 64U;
// Размер заголовка записи (magic, state, seq, crc, …).
constexpr size_t kRecordHeaderSize = 12U;
// Минимальный запас свободного места в секторе; при меньшем остатке вызовется compact() при следующей загрузке.
constexpr size_t kCompactThreshold = 100U * kRecordSize;

// Ожидаемые поля PersistentStorage при проверке CRC.
constexpr uint8_t kStorageId = 0xD1U;
constexpr uint8_t kStorageVersion = 3U;
constexpr uint8_t kCrc8Polynomial = 0x07U;

#ifndef FLASH_FLAG_ALL_ERRORS
// Маска флагов ошибок flash для очистки перед программированием.
#define FLASH_FLAG_ALL_ERRORS                                                                 \
  (FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |                \
   FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR)
#endif

static_assert(sizeof(ConfigManager::PersistentStorage) == 41,
              "PersistentStorage layout mismatch");

template <typename StorageType>
uint8_t calculateStorageCrc8(const StorageType& storage) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&storage);
  const size_t length = offsetof(StorageType, crc8);
  uint8_t crc = 0;

  for (size_t i = 0; i < length; ++i) {
    crc ^= bytes[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      if ((crc & 0x80u) != 0u) {
        crc = static_cast<uint8_t>((crc << 1u) ^ kCrc8Polynomial);
      } else {
        crc = static_cast<uint8_t>(crc << 1u);
      }
    }
  }

  return crc;
}

uint8_t headerXor(const uint8_t* header) {
  uint8_t value = 0;
  for (size_t i = 0; i < 11; ++i) {
    value ^= header[i];
  }
  return value;
}

bool isValidStorageV3(const ConfigManager::PersistentStorage& storage) {
  if (storage.id != kStorageId) {
    return false;
  }
  if (storage.version != kStorageVersion) {
    return false;
  }
  return calculateStorageCrc8(storage) == storage.crc8;
}

enum class ProgramResult : uint8_t {
  Ok,
  UnlockFailed,
  ProgramFailed,
};

ProgramResult programFlashWords(uint32_t address, const uint8_t* data, size_t length) {
  if ((address & 0x3u) != 0u || (length & 0x3u) != 0u) {
    return ProgramResult::ProgramFailed;
  }

  if (HAL_FLASH_Unlock() != HAL_OK) {
    return ProgramResult::UnlockFailed;
  }

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

  bool ok = true;
  __disable_irq();
  for (size_t offset = 0; offset < length; offset += 4U) {
    uint32_t word = 0;
    memcpy(&word, data + offset, sizeof(word));
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + offset, word) != HAL_OK) {
      ok = false;
      break;
    }
  }
  __enable_irq();

  HAL_FLASH_Lock();
  return ok ? ProgramResult::Ok : ProgramResult::ProgramFailed;
}

}  // namespace

FlashConfigStore& FlashConfigStore::instance() {
  static FlashConfigStore store;
  return store;
}

bool FlashConfigStore::begin() {
  scan();
  m_initialized = true;
  return true;
}

bool FlashConfigStore::isRecordErased(const uint8_t* record_bytes) const {
  for (size_t i = 0; i < 8; ++i) {
    if (record_bytes[i] != 0xFFu) {
      return false;
    }
  }
  return true;
}

bool FlashConfigStore::isRecordValid(const uint8_t* record_bytes,
                                     ConfigManager::PersistentStorage& out) const {
  if (invalidReason(record_bytes) != InvalidReason::None) {
    return false;
  }

  memcpy(&out, record_bytes + kRecordHeaderSize, sizeof(out));
  return isValidStorageV3(out);
}

FlashConfigStore::InvalidReason FlashConfigStore::invalidReason(
    const uint8_t* record_bytes) const {
  const uint16_t magic =
      static_cast<uint16_t>(record_bytes[0]) |
      (static_cast<uint16_t>(record_bytes[1]) << 8);

  if (magic != kRecordMagic) {
    return InvalidReason::BadMagic;
  }
  if (record_bytes[2] != kRecordStateValid) {
    return InvalidReason::BadState;
  }
  if (record_bytes[3] != kRecordFormatVersion) {
    return InvalidReason::BadFormat;
  }
  if (headerXor(record_bytes) != record_bytes[11]) {
    return InvalidReason::BadHeaderXor;
  }

  const uint16_t payload_size =
      static_cast<uint16_t>(record_bytes[8]) |
      (static_cast<uint16_t>(record_bytes[9]) << 8);
  if (payload_size != sizeof(ConfigManager::PersistentStorage)) {
    return InvalidReason::BadPayloadSize;
  }

  ConfigManager::PersistentStorage payload{};
  memcpy(&payload, record_bytes + kRecordHeaderSize, sizeof(payload));

  const uint8_t expected_crc = calculateStorageCrc8(payload);
  if (record_bytes[10] != expected_crc || payload.crc8 != expected_crc) {
    return InvalidReason::BadPayloadCrc;
  }

  if (!isValidStorageV3(payload)) {
    return InvalidReason::BadStorageMeta;
  }

  return InvalidReason::None;
}

void FlashConfigStore::scan() {
  m_next_offset = 0;
  m_last_seq = 0;

  uint32_t best_seq = 0;
  bool found_valid = false;

  for (uint32_t offset = 0; offset + kRecordSize <= kSectorSize; offset += kRecordSize) {
    const uint8_t* record_bytes =
        reinterpret_cast<const uint8_t*>(kSectorBase + offset);

    if (isRecordErased(record_bytes)) {
      m_next_offset = offset;
      break;
    }

    ConfigManager::PersistentStorage candidate{};
    if (!isRecordValid(record_bytes, candidate)) {
      continue;
    }

    const uint32_t seq =
        static_cast<uint32_t>(record_bytes[4]) |
        (static_cast<uint32_t>(record_bytes[5]) << 8) |
        (static_cast<uint32_t>(record_bytes[6]) << 16) |
        (static_cast<uint32_t>(record_bytes[7]) << 24);

    if (!found_valid || seq >= best_seq) {
      best_seq = seq;
      found_valid = true;
    }

    m_next_offset = offset + kRecordSize;
  }

  if (found_valid) {
    m_last_seq = best_seq;
  }

  if (m_next_offset > kSectorSize - kRecordSize) {
    m_next_offset = kSectorSize;
  }
}

bool FlashConfigStore::load(ConfigManager::PersistentStorage& out) {
  uint32_t best_seq = 0;
  bool found_valid = false;

  for (uint32_t offset = 0; offset + kRecordSize <= kSectorSize; offset += kRecordSize) {
    const uint8_t* record_bytes =
        reinterpret_cast<const uint8_t*>(kSectorBase + offset);

    if (isRecordErased(record_bytes)) {
      break;
    }

    ConfigManager::PersistentStorage candidate{};
    if (!isRecordValid(record_bytes, candidate)) {
      continue;
    }

    const uint32_t seq =
        static_cast<uint32_t>(record_bytes[4]) |
        (static_cast<uint32_t>(record_bytes[5]) << 8) |
        (static_cast<uint32_t>(record_bytes[6]) << 16) |
        (static_cast<uint32_t>(record_bytes[7]) << 24);

    if (!found_valid || seq >= best_seq) {
      best_seq = seq;
      out = candidate;
      found_valid = true;
    }
  }

  if (found_valid) {
    m_last_seq = best_seq;
  }

  scan();
  m_initialized = true;
  return found_valid;
}

bool FlashConfigStore::appendRecord(const ConfigManager::PersistentStorage& storage,
                                    uint32_t seq) {
  if (m_next_offset + kRecordSize > kSectorSize) {
    return false;
  }

  alignas(8) static uint8_t record[kRecordSize];
  memset(record, 0xFF, sizeof(record));

  record[0] = static_cast<uint8_t>(kRecordMagic & 0xFFu);
  record[1] = static_cast<uint8_t>((kRecordMagic >> 8) & 0xFFu);
  record[2] = kRecordStateValid;
  record[3] = kRecordFormatVersion;
  record[4] = static_cast<uint8_t>(seq & 0xFFu);
  record[5] = static_cast<uint8_t>((seq >> 8) & 0xFFu);
  record[6] = static_cast<uint8_t>((seq >> 16) & 0xFFu);
  record[7] = static_cast<uint8_t>((seq >> 24) & 0xFFu);
  record[8] = static_cast<uint8_t>(sizeof(ConfigManager::PersistentStorage) & 0xFFu);
  record[9] = static_cast<uint8_t>((sizeof(ConfigManager::PersistentStorage) >> 8) & 0xFFu);
  record[10] = calculateStorageCrc8(storage);
  record[11] = headerXor(record);
  memcpy(record + kRecordHeaderSize, &storage, sizeof(storage));

  const uint32_t address = kSectorBase + m_next_offset;
  if (programFlashWords(address, record, kRecordSize) != ProgramResult::Ok) {
    return false;
  }

  const uint8_t* programmed = reinterpret_cast<const uint8_t*>(address);
  ConfigManager::PersistentStorage verify{};
  if (!isRecordValid(programmed, verify)) {
    return false;
  }
  if (memcmp(&verify, &storage, sizeof(storage)) != 0) {
    return false;
  }

  m_next_offset += kRecordSize;
  m_last_seq = seq;
  return true;
}

bool FlashConfigStore::save(const ConfigManager::PersistentStorage& storage) {
  if (!m_initialized) {
    begin();
  }

  const uint32_t seq = m_last_seq + 1U;
  return appendRecord(storage, seq);
}

bool FlashConfigStore::needsCompaction() const {
  if (kSectorSize <= m_next_offset) {
    return true;
  }
  return (kSectorSize - m_next_offset) < kCompactThreshold;
}

bool FlashConfigStore::eraseSector() {
  FLASH_EraseInitTypeDef erase{};
  uint32_t sector_error = 0;

  erase.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  erase.Sector = FLASH_CONFIG_SECTOR;
  erase.NbSectors = 1;

  if (HAL_FLASH_Unlock() != HAL_OK) {
    return false;
  }

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  __disable_irq();
  const HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &sector_error);
  __enable_irq();
  HAL_FLASH_Lock();

  return status == HAL_OK;
}

bool FlashConfigStore::compact(const ConfigManager::PersistentStorage& latest) {
  PROFILE_SCOPE("FlashConfigCompact");

  if (!eraseSector()) {
    return false;
  }

  m_next_offset = 0;
  m_last_seq = 0;

  if (!appendRecord(latest, 1U)) {
    return false;
  }

  return true;
}

uint32_t FlashConfigStore::sectorCapacityBytes() {
  return kSectorSize;
}

uint32_t FlashConfigStore::recordSizeBytes() {
  return static_cast<uint32_t>(kRecordSize);
}

uint32_t FlashConfigStore::recordCount() const {
  uint32_t count = 0;

  for (uint32_t offset = 0; offset + kRecordSize <= kSectorSize; offset += kRecordSize) {
    const uint8_t* record_bytes =
        reinterpret_cast<const uint8_t*>(kSectorBase + offset);

    if (isRecordErased(record_bytes)) {
      break;
    }

    if (invalidReason(record_bytes) == InvalidReason::None) {
      ++count;
    }
  }

  return count;
}

uint32_t FlashConfigStore::freeRecordSlots() const {
  if (m_next_offset + kRecordSize > kSectorSize) {
    return 0;
  }

  return (kSectorSize - m_next_offset) / static_cast<uint32_t>(kRecordSize);
}

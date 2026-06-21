#pragma once

#include <stdint.h>

#include "system/ConfigManager.h"

class FlashConfigStore {
public:
  static FlashConfigStore& instance();

  bool begin();
  bool load(ConfigManager::PersistentStorage& out);
  bool save(const ConfigManager::PersistentStorage& storage);

  bool needsCompaction() const;
  bool compact(const ConfigManager::PersistentStorage& latest);

private:
  FlashConfigStore() = default;

  enum class InvalidReason : uint8_t {
    None,
    BadMagic,
    BadState,
    BadFormat,
    BadHeaderXor,
    BadPayloadSize,
    BadPayloadCrc,
    BadStorageMeta,
  };

  void scan();
  bool appendRecord(const ConfigManager::PersistentStorage& storage, uint32_t seq);
  bool eraseSector();
  bool isRecordValid(const uint8_t* record_bytes, ConfigManager::PersistentStorage& out) const;
  bool isRecordErased(const uint8_t* record_bytes) const;
  InvalidReason invalidReason(const uint8_t* record_bytes) const;

  bool m_initialized = false;
  uint32_t m_next_offset = 0;
  uint32_t m_last_seq = 0;
};

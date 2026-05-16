#include "display/assets/icons.h"

// ─── Заглушки иконок ──────────────────────────────────────────────────────────
// Все указатели установлены в nullptr.
// Виджеты проверяют != nullptr перед вызовом drawBitmap() — падения не будет.
//
// Чтобы добавить реальные данные, замени nullptr на массив:
//
//   static const uint8_t icon_temp_low_data[] = { 0x3C, 0x42, ... };
//   const uint8_t* const ICON_TEMP_LOW = icon_temp_low_data;

const uint8_t* const ICON_TEMP_LOW   = nullptr;
const uint8_t* const ICON_DROPLET_PCT = nullptr;
const uint8_t* const ICON_PUMP_OUTER = nullptr;
const uint8_t* const ICON_PUMP_INNER = nullptr;
const uint8_t* const ICON_IGNITION   = nullptr;

#pragma once
#include <stdint.h>

// Иконки как VLW-шрифты (Font Awesome 7 Pro).
// Каждый размер — отдельный шрифт; глифы адресуются UTF-8 кодами.

namespace icon {

namespace large {
    extern const uint8_t* const font;

    static constexpr int16_t GLYPH_SIZE = 44;
    static constexpr int16_t BOX_W      = 44;
    static constexpr int16_t BOX_H      = 44;

    extern const char* const POWER;
    extern const char* const UP;
    extern const char* const DROPLET;
    extern const char* const FIRE;
    extern const char* const DOWN;
}

namespace medium {
    extern const uint8_t* const font;

    static constexpr int16_t GLYPH_SIZE = 24;
    static constexpr int16_t BOX_W      = 24;
    static constexpr int16_t BOX_H      = 24;

    extern const char* const DROPLET_PCT;
    extern const char* const TEMP;
}

namespace small {
    extern const uint8_t* const font;

    static constexpr int16_t GLYPH_SIZE = 14;
    static constexpr int16_t BOX_W      = 14;
    static constexpr int16_t BOX_H      = 14;

    extern const char* const SCRUBBER;
    extern const char* const POWER_OFF;
}

}  // namespace icon

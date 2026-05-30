#pragma once

#include <stdint.h>

class TFT_eSPI;
class TFT_eSprite;

enum class SpriteSlot : uint8_t { Large, Medium, Small }; 

struct SpriteScreenRect {
    int16_t    x;
    int16_t    y;
    SpriteSlot slot;
};

// Пул предвыделенных TFT_eSprite для HomeScreen (создаётся в onEnter, освобождается в onExit).
class SpritePool {
public:
    static constexpr int16_t LARGE_W  = 145;
    static constexpr int16_t LARGE_H  = 51;

    static constexpr int16_t MEDIUM_W = 85;
    static constexpr int16_t MEDIUM_H = 25;
    
    static constexpr int16_t SMALL_W  = 32;
    static constexpr int16_t SMALL_H  = 44;

    static constexpr uint16_t OOM_MARKER_COLOR = 0xF800;  // red RGB565

    static void init(TFT_eSPI& tft);
    static void shutdown();

    // nullptr если слот не создан при init (OOM).
    static TFT_eSprite* acquire(SpriteSlot slot);
    static void release(SpriteSlot slot);

    static bool isReady(SpriteSlot slot);

    static int16_t slotWidth(SpriteSlot slot);
    static int16_t slotHeight(SpriteSlot slot);

    // Красная обводка — видимый индикатор нехватки RAM.
    static void drawOomMarker(TFT_eSPI& tft, const SpriteScreenRect& rect);
};

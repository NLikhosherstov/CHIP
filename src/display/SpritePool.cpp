#include "display/SpritePool.h"

#include <TFT_eSPI.h>

namespace {
    TFT_eSprite* s_sprites[3] = {nullptr, nullptr, nullptr};
    bool         s_ready[3]   = {false, false, false};
    bool         s_inUse[3]   = {false, false, false};

    int slotIndex(SpriteSlot slot) {
        return static_cast<int>(slot);
    }

    void destroySlot(int i) {
        if (s_sprites[i] != nullptr) {
            s_sprites[i]->deleteSprite();
            delete s_sprites[i];
            s_sprites[i] = nullptr;
        }
        s_ready[i] = false;
        s_inUse[i] = false;
    }

    bool createSlot(TFT_eSPI& tft, int i, int16_t w, int16_t h) {
        destroySlot(i);
        s_sprites[i] = new TFT_eSprite(&tft);
        s_ready[i]   = s_sprites[i]->createSprite(w, h);
        if (!s_ready[i]) {
            destroySlot(i);
            return false;
        }
        return true;
    }
}  // namespace

void SpritePool::init(TFT_eSPI& tft) {
    shutdown();
    createSlot(tft, slotIndex(SpriteSlot::Large), LARGE_W, LARGE_H);
    createSlot(tft, slotIndex(SpriteSlot::Medium), MEDIUM_W, MEDIUM_H);
    createSlot(tft, slotIndex(SpriteSlot::Small), SMALL_W, SMALL_H);
}

void SpritePool::shutdown() {
    for (int i = 0; i < 3; ++i) {
        destroySlot(i);
    }
}

TFT_eSprite* SpritePool::acquire(SpriteSlot slot) {
    const int i = slotIndex(slot);
    if (!s_ready[i] || s_inUse[i] || s_sprites[i] == nullptr) {
        return nullptr;
    }
    s_inUse[i] = true;
    return s_sprites[i];
}

void SpritePool::release(SpriteSlot slot) {
    s_inUse[slotIndex(slot)] = false;
}

bool SpritePool::isReady(SpriteSlot slot) {
    return s_ready[slotIndex(slot)];
}

int16_t SpritePool::slotWidth(SpriteSlot slot) {
    switch (slot) {
        case SpriteSlot::Large:  return LARGE_W;
        case SpriteSlot::Medium: return MEDIUM_W;
        case SpriteSlot::Small:  return SMALL_W;
    }
    return 0;
}

int16_t SpritePool::slotHeight(SpriteSlot slot) {
    switch (slot) {
        case SpriteSlot::Large:  return LARGE_H;
        case SpriteSlot::Medium: return MEDIUM_H;
        case SpriteSlot::Small:  return SMALL_H;
    }
    return 0;
}

void SpritePool::drawOomMarker(TFT_eSPI& tft, const SpriteScreenRect& rect) {
    tft.drawRect(rect.x, rect.y, slotWidth(rect.slot), slotHeight(rect.slot), OOM_MARKER_COLOR);
}

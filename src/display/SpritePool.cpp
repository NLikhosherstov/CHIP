#include "display/SpritePool.h"

#include <TFT_eSPI.h>

namespace {
    TFT_eSprite* s_sprites[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    bool         s_ready[5]   = {false, false, false, false, false};
    bool         s_inUse[5]   = {false, false, false, false, false};

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
    createSlot(tft, slotIndex(SpriteSlot::XL), XL_W, XL_H);
    createSlot(tft, slotIndex(SpriteSlot::L), L_W, L_H);
    createSlot(tft, slotIndex(SpriteSlot::M), M_W, M_H);
    createSlot(tft, slotIndex(SpriteSlot::S), S_W, S_H);
    createSlot(tft, slotIndex(SpriteSlot::XS), XS_W, XS_H);
}

void SpritePool::shutdown() {
    for (int i = 0; i < 5; ++i) {
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
        case SpriteSlot::XL:  return XL_W;
        case SpriteSlot::L:   return L_W;
        case SpriteSlot::M:   return M_W;
        case SpriteSlot::S:   return S_W;
        case SpriteSlot::XS:  return XS_W;
    }
    return 0;
}

int16_t SpritePool::slotHeight(SpriteSlot slot) {
    switch (slot) {
        case SpriteSlot::XL:  return XL_H;
        case SpriteSlot::L:   return L_H;
        case SpriteSlot::M:   return M_H;
        case SpriteSlot::S:   return S_H;
        case SpriteSlot::XS:  return XS_H;
    }
    return 0;
}

void SpritePool::drawOomMarker(TFT_eSPI& tft, const SpriteScreenRect& rect) {
    tft.drawRect(rect.x, rect.y, slotWidth(rect.slot), slotHeight(rect.slot), OOM_MARKER_COLOR);
}

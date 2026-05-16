#include "display/widgets/ModeWidget.h"

#include <TFT_eSPI.h>
#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"

void ModeWidget::draw(TFT_eSPI& tft,
                      SystemState::AutomationState state,
                      const PaletteRGB565& pal) {
    tft.fillRect(X, Y, W, H, pal.screenBg);
    update(tft, state, pal);
}

void ModeWidget::update(TFT_eSPI& tft,
                        SystemState::AutomationState state,
                        const PaletteRGB565& pal) {
    tft.fillRect(X, Y, W, H, pal.screenBg);

    // Название режима (крупно, Aldrich 24px)
    tft.setFreeFont(Font_Aldrich_24);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(pal.modeNameColor, pal.screenBg);
    tft.drawString(modeName(state), X, Y);

    // Подпись "режим" (мелко, Roboto Mono 12px)
    tft.setFreeFont(Font_RobotoMono_12);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.drawString("режим", X + 1, Y + 22);

    tft.setTextFont(1);
}

const char* ModeWidget::modeName(SystemState::AutomationState state) {
    // ASCII-идентификаторы режимов (безопасны при любой кодировке шрифта).
    // При использовании шрифта с кириллицей — заменить на localised строки.
    switch (state) {
        case SystemState::AutomationState::STATE_IDLE:       return "IDLE";
        case SystemState::AutomationState::STATE_AUTO_START: return "AUTO";
        case SystemState::AutomationState::STATE_AUTO_TSTAT: return "AUTO";
        case SystemState::AutomationState::STATE_MANUAL:     return "MANU";
        case SystemState::AutomationState::STATE_STOP:       return "STOP";
        default:                                             return "----";
    }
}

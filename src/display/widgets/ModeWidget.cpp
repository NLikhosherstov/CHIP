#include "display/widgets/ModeWidget.h"

#include <TFT_eSPI.h>
#include "display/PaletteRGB565.h"
#include "display/assets/fonts.h"

void ModeWidget::draw(TFT_eSPI& tft,
                      SystemState::AutomationState state,
                      const PaletteRGB565& pal) {
    update(tft, state, pal);
}

void ModeWidget::update(TFT_eSPI& tft,
                        SystemState::AutomationState state,
                        const PaletteRGB565& pal) {
    tft.fillRect(X, 0, W, H+Y, pal.screenBg);

    // Название режима
    tft.loadFont(smooth_font::def);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(pal.modeNameColor, pal.screenBg);
    tft.drawString(modeName(state), X, Y);
    tft.unloadFont();

    // Подпись
    tft.loadFont(smooth_font::small);
    tft.setTextColor(pal.labelColor, pal.screenBg);
    tft.drawString("РЕЖИМ", X, Y + 22);
    tft.unloadFont();

    tft.setTextFont(1);
}

const char* ModeWidget::modeName(SystemState::AutomationState state) {
    // ASCII-идентификаторы режимов (безопасны при любой кодировке шрифта).
    // При использовании шрифта с кириллицей — заменить на localised строки.
    switch (state) {
        case SystemState::AutomationState::STATE_IDLE:       return "ГОТОВ";
        case SystemState::AutomationState::STATE_AUTO_START: return "АВТО";
        case SystemState::AutomationState::STATE_AUTO_TSTAT: return "АВТО";
        case SystemState::AutomationState::STATE_MANUAL:     return "РУЧНОЙ";
        case SystemState::AutomationState::STATE_STOP:       return "СТОП";
        default:                                             return "----";
    }
}

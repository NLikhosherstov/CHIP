#include "display/widgets/IgnitionIndicator.h"

#include <TFT_eSPI.h>
#include "display/PaletteRGB565.h"
#include "display/assets/icons.h"

void IgnitionIndicator::draw(TFT_eSPI& tft,
                              bool active,
                              const PaletteRGB565& pal) {
    const uint16_t color = active ? pal.ignitionActiveColor : pal.ignitionIdleColor;
    const int16_t ix = CX - W / 2;
    const int16_t iy = CY - H / 2;

    if (ICON_IGNITION != nullptr) {
        tft.drawBitmap(ix, iy, ICON_IGNITION, ICON_IGNITION_W, ICON_IGNITION_H,
                       color, pal.screenBg);
    } else {
        // Заглушка: прямоугольник-индикатор до появления реальной иконки
        tft.fillRect(ix, iy, W, H, active ? color : pal.screenBg);
        if (!active) {
            tft.drawRect(ix, iy, W, H, pal.ignitionIdleColor);
        }
    }
}

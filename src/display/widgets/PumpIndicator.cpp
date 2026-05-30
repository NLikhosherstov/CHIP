#include "display/widgets/PumpIndicator.h"

#include <TFT_eSPI.h>

#include "display/IconDraw.h"
#include "display/PaletteRGB565.h"
#include "display/assets/icons.h"

void PumpIndicator::draw(TFT_eSPI& tft, bool active, const PaletteRGB565& pal) {
    const uint16_t outerColor = active ? pal.pumpIconActiveColor : pal.pumpIconIdleColor;
    IconDraw::drawLayeredBitmap(tft, X, Y,
                                icon::PUMP_PRIMARY, icon::PUMP_SECONDARY,
                                icon::PUMP_W, icon::PUMP_H,
                                outerColor);
}


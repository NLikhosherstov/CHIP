#include "display/widgets/IgnitionIndicator.h"

#include <TFT_eSPI.h>

#include "display/IconDraw.h"
#include "display/PaletteRGB565.h"
#include "display/assets/icons.h"

void IgnitionIndicator::draw(TFT_eSPI& tft, bool active, const PaletteRGB565& pal) {
    const uint16_t outerColor = active ? pal.ignitionActiveColor : pal.ignitionIdleColor;
    IconDraw::drawLayeredBitmap(tft, X, Y,
                                icon::IGNITION_PRIMARY, icon::IGNITION_SECONDARY,
                                icon::IGNITION_W, icon::IGNITION_H,
                                outerColor);
}

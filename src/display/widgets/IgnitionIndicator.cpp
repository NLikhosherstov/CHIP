#include "display/widgets/IgnitionIndicator.h"

#include <TFT_eSPI.h>

#include "display/IconDraw.h"
#include "display/PaletteRGB565.h"
#include "display/assets/icons.h"

void IgnitionIndicator::draw(TFT_eSPI& tft, bool active, const PaletteRGB565& pal) {
    (void)pal;
    const uint16_t color = active ? CLR_IGN_ICON : CLR_ICON_INACTIVE;

    IconDraw::drawIcon(tft,
                       X + icon::large::BOX_W / 2,
                       Y + icon::large::BOX_H / 2,
                       icon::large::font,
                       icon::large::FIRE,
                       color,
                       CLR_BG);
}

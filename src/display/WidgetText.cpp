#include "display/WidgetText.h"

#include <string.h>

#include <TFT_eSPI.h>

namespace WidgetText {

bool textChanged(const char* newStr, char* lastStr, const size_t lastStrSize) {
    if (newStr == nullptr || lastStr == nullptr || lastStrSize == 0) {
        return false;
    }
    if (strcmp(newStr, lastStr) == 0) {
        return false;
    }
    strncpy(lastStr, newStr, lastStrSize - 1);
    lastStr[lastStrSize - 1] = '\0';
    return true;
}

void fillRect(TFT_eSPI& tft, const int16_t x, const int16_t y,
              const int16_t w, const int16_t h, const uint16_t color) {
    tft.startWrite();
    tft.fillRect(x, y, w, h, color);
    tft.endWrite();
}

}  // namespace WidgetText

#pragma once

#include <stddef.h>
#include <stdint.h>

class TFT_eSPI;

// Общие хелперы для перерисовки текстовых полей без лишнего SPI-трафика.
namespace WidgetText {

// true, если newStr отличается от lastStr (lastStr обновляется при true).
bool textChanged(const char* newStr, char* lastStr, size_t lastStrSize);

// Заливка прямоугольника с батчингом SPI.
void fillRect(TFT_eSPI& tft, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

}  // namespace WidgetText

#include "display.h"

#include <TFT_eSPI.h>

namespace {
  constexpr uint8_t kBacklightPin = PB10;
}

Display::Display() : tft_(new TFT_eSPI()) {
  pinMode(kBacklightPin, OUTPUT);
  backlightOff();

  tft_->setRotation(3);
  tft_->init();
  tft_->fillScreen(TFT_BLACK);

  tft_->setTextColor(TFT_YELLOW, TFT_BLACK);
  tft_->drawCentreString("CHIP v.0.1", 160, 50, 4);

  tft_->setTextColor(TFT_WHITE, TFT_BLACK);
  tft_->drawCentreString("Heater control unit", 160, 100, 2);

  tft_->drawRect(20, 150, 280, 20, TFT_BLUE);
  tft_->setTextColor(TFT_GREEN);
  tft_->drawCentreString("READY", 160, 180, 2);

  backlightOn();
}

Display::~Display() {
  delete tft_;
}

void Display::backlightOn() {
  digitalWrite(kBacklightPin, HIGH);
}

void Display::backlightOff() {
  digitalWrite(kBacklightPin, LOW);
}

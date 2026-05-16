#include "display.h"

#include <TFT_eSPI.h>
#include "pins.h"

namespace {
}

Display::Display() : m_tft(new TFT_eSPI()) {
  pinMode(pin::DISPLAY_LIGHT, OUTPUT);
  backlightOff();

  m_tft->setRotation(3);
  m_tft->init();
  m_tft->fillScreen(TFT_BLACK);

  m_tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  m_tft->drawCentreString("CHIP v.0.1", 160, 50, 4);

  m_tft->setTextColor(TFT_WHITE, TFT_BLACK);
  m_tft->drawCentreString("Heater control unit", 160, 100, 2);

  m_tft->drawRect(20, 150, 280, 20, TFT_BLUE);
  m_tft->setTextColor(TFT_GREEN);
  m_tft->drawCentreString("READY", 160, 180, 2);

  backlightOn();
}

Display::~Display() {
  delete m_tft;
}

void Display::backlightOn() {
  digitalWrite(pin::DISPLAY_LIGHT, HIGH);
}

void Display::backlightOff() {
  digitalWrite(pin::DISPLAY_LIGHT, LOW);
}

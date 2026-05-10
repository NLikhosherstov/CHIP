#pragma once

#include <Arduino.h>

class TFT_eSPI;

class Display {
public:
  Display();
  ~Display();

  void backlightOn();
  void backlightOff();

private:
  TFT_eSPI* tft_;
};

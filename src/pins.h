#pragma once

#include <Arduino.h>


namespace pin{

// Display
constexpr uint8_t DISPLAY_LIGHT = PB10;

// Encoder (ENC)
constexpr uint8_t ENC_S1 = PB8;   // Pin 4
constexpr uint8_t ENC_S2 = PB9;   // Pin 3
constexpr uint8_t ENC_KEY = PB7;  // Pin 2

// Keyboard (KBD)
constexpr uint8_t KBD_OUT = PB0;  // Analog keyboard input

// Temperature/humidity sensor (DHT)
constexpr uint8_t DHT_DATA = PB11;  // Pin 2

// Heat exchanger temperature sensor (MAX31855)
constexpr uint8_t MAX_CS = PB12;   // Pin 5
constexpr uint8_t MAX_CLK = PB13;  // Pin 6
constexpr uint8_t MAX_DO = PB14;   // Pin 4

// Actuators
constexpr uint8_t MOTOR = PA8;    // Motor control
constexpr uint8_t IGNITOR = PA9;  // Ignitor control
constexpr uint8_t PUMP  = PA10;   // Fuel pump control

}
#pragma once

#include "PinNames.h"
#include <Arduino.h>


namespace pin{

// Display
constexpr PinName DISPLAY_LIGHT = PB_1;

// Encoder (ENC)
constexpr PinName ENC_S1 = PB_8;   // Pin 4
constexpr PinName ENC_S2 = PB_9;   // Pin 3
constexpr PinName ENC_KEY = PB_7;  // Pin 2

// Keyboard (KBD)
constexpr PinName KBD_OUT = PB_0;  // Analog keyboard input

// Temperature/humidity sensor (DHT)
constexpr PinName DHT_DATA = PA_6;  // Pin 2

// Heat exchanger temperature sensor (MAX31855)
constexpr PinName MAX_CS = PB_12;   // Pin 5
constexpr PinName MAX_CLK = PB_13;  // Pin 6
constexpr PinName MAX_DO = PB_14;   // Pin 4

// Actuators
constexpr PinName MOTOR = PA_8;    // Motor control
constexpr PinName IGNITOR = PA_9;  // Ignitor control
constexpr PinName PUMP  = PA_10;   // Fuel pump control

}

namespace aPin{
    constexpr uint32_t KBD_OUT = A8;  // Analog keyboard input
}
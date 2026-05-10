#include <Arduino.h>
#include "display.h"
#define PIN_MOTOR   PA8
#define PIN_IGNITOR PA9
#define PIN_PUMP    PA10

void setup() {
  Serial.begin(115200);
  pinMode(PC13, OUTPUT); // Встроенный светодиод

  static Display display;
  (void)display;

  // Инициализация пинов в режим выходов
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_IGNITOR, OUTPUT);
  pinMode(PIN_PUMP, OUTPUT);
  
  // Принудительное выключение при старте
  digitalWrite(PIN_MOTOR, LOW);
  digitalWrite(PIN_IGNITOR, LOW);
  digitalWrite(PIN_PUMP, LOW);
}

void loop() {
  // 1. Проверка силовой цепи двигателя
  digitalWrite(PIN_MOTOR, HIGH);
  delay(800);
  digitalWrite(PIN_MOTOR, LOW);
  
  // 2. Проверка силовой цепи свечи
  digitalWrite(PIN_IGNITOR, HIGH);
  delay(800);
  digitalWrite(PIN_IGNITOR, LOW);
  
  // 3. Проверка силовой цепи насоса
  digitalWrite(PIN_PUMP, HIGH);
  delay(800);
  digitalWrite(PIN_PUMP, LOW);
}
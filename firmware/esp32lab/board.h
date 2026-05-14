#pragma once
#include <Arduino.h>

void   boardSetup();
String boardName();
int    boardGpioMax();
bool   boardPinReserved(int pin);
int    boardDefaultLedPin();
int    boardLedOn();            // HIGH or LOW
int    boardBootPin();          // BOOT button GPIO (active LOW), or -1 if unknown

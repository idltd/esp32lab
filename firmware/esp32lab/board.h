#pragma once
#include <Arduino.h>

void   boardSetup();
String boardName();
int    boardGpioMax();
bool   boardPinReserved(int pin);
int    boardDefaultLedPin();
int    boardLedOn();            // HIGH or LOW

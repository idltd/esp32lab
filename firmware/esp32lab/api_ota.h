#pragma once
void setupOtaApi();
void otaLoop();       // call from loop() — executes pending restart after TCP flushes

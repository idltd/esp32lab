#pragma once

void   setupSystemApi();
void   systemLoop();      // call from loop() — handles triple-click factory reset
String getDeviceName();   // MAC-based default, overridden by NVS "devname"

@echo off
py "C:\Users\paul\.platformio\packages\tool-esptoolpy\esptool.py" --chip esp32c3 write_flash 0x0 firmware\build\esp32c3\esp32lab.ino.merged.bin

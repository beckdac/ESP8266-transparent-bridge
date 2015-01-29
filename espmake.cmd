@echo off
REM see "New Windows terminal/flasher apps & Visual Studio" http://www.esp8266.com/viewtopic.php?f=9&t=911 to setup Visual Studio 2013

REM remove automatic created obj folder
rd obj /S /Q >nul 2>&1

PATH=%PATH%;%~dp0..\xtensa-lx106-elf\bin;C:\MinGW\bin;C:\MinGW\msys\1.0\bin
make  -f makefile_VS2013 %1 %2 %3 %4 %5


@echo off
REM
REM patch environment config
REM
set PATH=C:\w64devkit\bin
make -f Makefile.win32 clean
make -f Makefile.win32
pause

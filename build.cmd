@echo off
REM
REM patch environment config
REM
set PATH=C:\win-builds-patch-32\bin
gmake -f Makefile.win32 clean
gmake -f Makefile.win32
pause

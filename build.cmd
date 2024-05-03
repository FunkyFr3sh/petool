@echo off
REM
REM patch environment config
REM
set PATH=C:\w64devkit\bin;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\mingw64\bin\
make clean
make
pause

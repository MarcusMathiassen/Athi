@echo off

call cmake .. -G "Visual Studio 15 2017 Win64"
call cmake --build . -- -m

xcopy /S /Q /Y ..\bin\debug ..\bin
RMDIR /Q /S ..\bin\debug

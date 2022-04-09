@echo off

@REM Compile the project
@echo Compiling...
g++ -Wall -Wno-catch-value -Wno-unused-result -std=c++17 -Ofast -o asm main.cpp

@REM Copy the executable to the bin folder
@echo Copying...
copy /Y asm.exe "C:\Windows"

@REM Clean up
del asm.exe

pause

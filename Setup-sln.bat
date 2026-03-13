@echo off

REM Get the directory of the batch file
set SCRIPT_DIR=%~dp0

REM Navigate to the script directory
cd /d "%SCRIPT_DIR%"

print "%SCRIPT_DIR%"

REM Run Premake with the Lua file
%SCRIPT_DIR%Vendors\bin\premake\Windows\premake5.exe --file="%SCRIPT_DIR%reNut-Launcher-SetupScript.lua" vs2022
pause
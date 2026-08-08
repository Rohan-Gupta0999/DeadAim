@echo off
cd /d "%~dp0"

echo [run] Starting vision process...
start "DeadAim Vision" python vision\server.py

timeout /t 2 /nobreak >nul

echo [run] Starting game...
build\DeadAim.exe
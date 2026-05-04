@echo off
if not exist bin\omnisbelli.exe (
    call build.bat
)
echo [*] Launching Omnisbelli Planetary Siege Engine...
start bin\omnisbelli.exe

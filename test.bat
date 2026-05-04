@echo off
if not exist bin\test_simulation.exe (
    call build.bat
)
echo [*] Executing Omnisbelli Automated Test Suite...
bin\test_simulation.exe

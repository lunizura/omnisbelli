@echo off
setlocal enabledelayedexpansion

echo ==============================================================================
echo [OMNISBELLI] Compiling Planetary Siege Engine (MSVC x64)
echo ==============================================================================

where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo [*] Initializing MSVC x64 build tools...
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
)

if not exist bin mkdir bin

set CXX_FLAGS=/std:c++17 /EHsc /O2 /W4 /utf-8 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /Iinclude
set LIBS=user32.lib gdi32.lib

echo [*] Compiling Engine and Test Suite...
cl %CXX_FLAGS% /Fobin\ /Febin\test_simulation.exe ^
    tests\test_main.cpp ^
    src\engine\unit_class.cpp ^
    src\engine\army_roster.cpp ^
    src\engine\lanchester.cpp ^
    src\engine\synapse_network.cpp ^
    src\engine\logistics.cpp ^
    src\engine\sector.cpp ^
    src\engine\command_orders.cpp ^
    src\engine\projection.cpp ^
    src\engine\simulation.cpp ^
    src\engine\telemetry_export.cpp ^
    /link %LIBS%

if %errorlevel% neq 0 (
    echo [ERROR] Test suite compilation failed!
    exit /b %errorlevel%
)
echo [OK] bin\test_simulation.exe compiled successfully.

echo [*] Compiling Omnisbelli Win32 Application...
cl %CXX_FLAGS% /Fobin\ /Febin\omnisbelli.exe ^
    src\main.cpp ^
    src\engine\unit_class.cpp ^
    src\engine\army_roster.cpp ^
    src\engine\lanchester.cpp ^
    src\engine\synapse_network.cpp ^
    src\engine\logistics.cpp ^
    src\engine\sector.cpp ^
    src\engine\command_orders.cpp ^
    src\engine\projection.cpp ^
    src\engine\simulation.cpp ^
    src\gfx\framebuffer.cpp ^
    src\gfx\bitmap_font.cpp ^
    src\gfx\rasterizer.cpp ^
    src\gfx\crt_effects.cpp ^
    src\gfx\audio_sfx.cpp ^
    src\engine\telemetry_export.cpp ^
    src\ui\ui_manager.cpp ^
    src\ui\input_handler.cpp ^
    /link /SUBSYSTEM:WINDOWS %LIBS%

if %errorlevel% neq 0 (
    echo [ERROR] Omnisbelli Win32 compilation failed!
    exit /b %errorlevel%
)

echo [SUCCESS] bin\omnisbelli.exe built successfully!
echo ==============================================================================

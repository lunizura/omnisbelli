cmake_minimum_required(VERSION 3.15)
project(omnisbelli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8 /EHsc /O2)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
else()
    add_compile_options(-Wall -Wextra -O2)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ENGINE_SOURCES
    src/engine/unit_class.cpp
    src/engine/army_roster.cpp
    src/engine/lanchester.cpp
    src/engine/synapse_network.cpp
    src/engine/logistics.cpp
    src/engine/sector.cpp
    src/engine/command_orders.cpp
    src/engine/projection.cpp
    src/engine/simulation.cpp
    src/engine/telemetry_export.cpp
)

set(GFX_SOURCES
    src/gfx/framebuffer.cpp
    src/gfx/bitmap_font.cpp
    src/gfx/rasterizer.cpp
    src/gfx/crt_effects.cpp
    src/gfx/audio_sfx.cpp
)

set(UI_SOURCES
    src/ui/ui_manager.cpp
    src/ui/input_handler.cpp
)

# Main Application
add_executable(omnisbelli WIN32
    src/main.cpp
    ${ENGINE_SOURCES}
    ${GFX_SOURCES}
    ${UI_SOURCES}
)

if(WIN32)
    target_link_libraries(omnisbelli PRIVATE user32 gdi32)
endif()

# Automated Test Suite
add_executable(test_simulation
    tests/test_main.cpp
    ${ENGINE_SOURCES}
)

enable_testing()
add_test(NAME SimulationTests COMMAND test_simulation)

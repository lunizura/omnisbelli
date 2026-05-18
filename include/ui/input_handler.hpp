#pragma once
#include <windows.h>
#include "engine/simulation.hpp"
#include "ui/ui_manager.hpp"

namespace omnisbelli {

class InputHandler {
public:
    static bool handle_key_down(
        WPARAM wParam,
        SimulationEngine& sim,
        UIManager& ui
    );

    static bool handle_mouse_click(
        int virtual_x,
        int virtual_y,
        SimulationEngine& sim,
        UIManager& ui
    );
};

} // namespace omnisbelli

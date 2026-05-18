#include "ui/input_handler.hpp"

namespace omnisbelli {

bool InputHandler::handle_key_down(
    WPARAM wParam,
    SimulationEngine& sim,
    UIManager& ui
) {
    switch (wParam) {
    case VK_SPACE:
        sim.toggle_pause();
        return true;

    case 'P':
        ui.crt.cycle_palette();
        sim.add_event("[PALETTE] " + ui.crt.get_palette_name());
        return true;

    case 'C':
        ui.crt.toggle_scanlines();
        sim.add_event(ui.crt.scanlines_enabled ? "[CRT] SCANLINES ENGAGED." : "[CRT] SCANLINES DISENGAGED.");
        return true;

    case 'R':
        sim.reset();
        return true;

    case 'M':
        sim.run_planetary_projection(40);
        return true;

    case VK_TAB:
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            ui.customizer_toggle_side();
        } else {
            ui.cycle_tab();
        }
        return true;

    case 'L':
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            static int preset_cursor = 0;
            preset_cursor = (preset_cursor + 1) % 5;
            sim.load_scenario(preset_cursor);
            return true;
        }
        break;

    case '6':
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            sim.load_scenario(0);
            return true;
        }
        break;
    case '7':
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            sim.load_scenario(1);
            return true;
        }
        break;
    case '8':
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            sim.load_scenario(2);
            return true;
        }
        break;
    case '9':
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            sim.load_scenario(3);
            return true;
        }
        break;
    case '0':
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            sim.load_scenario(4);
            return true;
        }
        break;

    // Tabs 1-5 ALWAYS switch screens reliably!
    case '1':
        ui.switch_tab(ActiveScreenTab::TAB_TACTICAL);
        return true;
    case '2':
        ui.switch_tab(ActiveScreenTab::TAB_RADAR);
        return true;
    case '3':
        ui.switch_tab(ActiveScreenTab::TAB_LOGISTICS);
        return true;
    case '4':
        ui.switch_tab(ActiveScreenTab::TAB_CUSTOMIZER);
        return true;
    case '5':
        ui.switch_tab(ActiveScreenTab::TAB_PROJECTION);
        return true;

    // Speed controls
    case VK_OEM_PLUS:
    case VK_ADD:
    case VK_OEM_6: // ']'
        sim.set_speed(sim.time_multiplier * 2.0);
        return true;

    case VK_OEM_MINUS:
    case VK_SUBTRACT:
    case VK_OEM_4: // '['
        sim.set_speed(sim.time_multiplier * 0.5);
        return true;

    // Tactical Doctrines
    case VK_F1:
        sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
        return true;
    case VK_F2:
        sim.trigger_doctrine(DoctrineType::TITAN_PLASMA_VOLLEY);
        return true;
    case VK_F3:
        sim.trigger_doctrine(DoctrineType::ASTARTES_DROP_POD);
        return true;
    case VK_F4:
        sim.trigger_doctrine(DoctrineType::TACTICAL_FALLBACK);
        return true;
    case VK_F5:
        sim.trigger_doctrine(DoctrineType::ORBITAL_LANCE_STRIKE);
        return true;
    case VK_F8:
        sim.trigger_doctrine(DoctrineType::EXTERMINATUS_PROTOCOL);
        return true;

    // Customizer navigation
    case VK_UP:
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            ui.customizer_up();
            return true;
        }
        break;
    case VK_DOWN:
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            ui.customizer_down(sim);
            return true;
        }
        break;
    case VK_LEFT:
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            ui.customizer_adjust(sim, 0.5);
            return true;
        }
        break;
    case VK_RIGHT:
        if (ui.active_tab == ActiveScreenTab::TAB_CUSTOMIZER) {
            ui.customizer_adjust(sim, 2.0);
            return true;
        }
        break;

    default:
        break;
    }
    return false;
}

bool InputHandler::handle_mouse_click(
    int virtual_x,
    int virtual_y,
    SimulationEngine& sim,
    UIManager& ui
) {
    if (virtual_x < 0 || virtual_y < 0) return false;

    // Header bar tab clicks
    if (virtual_y >= 0 && virtual_y <= 24) {
        if (virtual_x >= 420 && virtual_x < 460) {
            ui.switch_tab(ActiveScreenTab::TAB_TACTICAL);
            return true;
        } else if (virtual_x >= 464 && virtual_x < 504) {
            ui.switch_tab(ActiveScreenTab::TAB_RADAR);
            return true;
        } else if (virtual_x >= 508 && virtual_x < 548) {
            ui.switch_tab(ActiveScreenTab::TAB_LOGISTICS);
            return true;
        } else if (virtual_x >= 552 && virtual_x < 592) {
            ui.switch_tab(ActiveScreenTab::TAB_CUSTOMIZER);
            return true;
        } else if (virtual_x >= 596 && virtual_x <= 638) {
            ui.switch_tab(ActiveScreenTab::TAB_PROJECTION);
            return true;
        } else if (virtual_x >= 188 && virtual_x < 410) {
            sim.toggle_pause();
            return true;
        }
    }

    // Tactical Screen doctrine button clicks
    if (ui.active_tab == ActiveScreenTab::TAB_TACTICAL && virtual_y >= 246 && virtual_y <= 320) {
        // Row 1: F1, F2, F3
        if (virtual_y >= 264 && virtual_y <= 288) {
            if (virtual_x >= 8 && virtual_x < 108) {
                sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
                return true;
            } else if (virtual_x >= 113 && virtual_x < 213) {
                sim.trigger_doctrine(DoctrineType::TITAN_PLASMA_VOLLEY);
                return true;
            } else if (virtual_x >= 218 && virtual_x < 318) {
                sim.trigger_doctrine(DoctrineType::ASTARTES_DROP_POD);
                return true;
            }
        }
        // Row 2: F4, F5, F8
        if (virtual_y >= 290 && virtual_y <= 314) {
            if (virtual_x >= 8 && virtual_x < 108) {
                sim.trigger_doctrine(DoctrineType::TACTICAL_FALLBACK);
                return true;
            } else if (virtual_x >= 113 && virtual_x < 213) {
                sim.trigger_doctrine(DoctrineType::ORBITAL_LANCE_STRIKE);
                return true;
            } else if (virtual_x >= 218 && virtual_x < 318) {
                sim.trigger_doctrine(DoctrineType::EXTERMINATUS_PROTOCOL);
                return true;
            }
        }
    }

    return false;
}

} // namespace omnisbelli

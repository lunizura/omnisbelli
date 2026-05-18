#pragma once
#include "engine/simulation.hpp"
#include "gfx/framebuffer.hpp"
#include "gfx/crt_effects.hpp"
#include "ui/ui_constants.hpp"
#include <vector>
#include <random>

namespace omnisbelli {

struct VisualParticle {
    float x, y;
    float vx, vy;
    float life;
    uint32_t color;
    int size;
};

class UIManager {
public:
    ActiveScreenTab active_tab;
    CRTEffects crt;

    // Unit customizer cursor state
    int customizer_selected_index; // 0..units.size()-1
    bool customizer_editing_swarm;  // false = human, true = swarm

    // Radar animated particles (gunfire tracers, artillery blasts, promethium flames)
    std::vector<VisualParticle> particles;
    std::mt19937 rng;

    UIManager();

    void update_visuals(double dt, const SimulationEngine& sim);
    void render(PixelFramebuffer& fb, const SimulationEngine& sim);

    void switch_tab(ActiveScreenTab tab);
    void cycle_tab();

    void customizer_up();
    void customizer_down(const SimulationEngine& sim);
    void customizer_adjust(SimulationEngine& sim, double factor);
    void customizer_toggle_side();

private:
    void render_header_bar(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme);
    void render_tactical_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme);
    void render_radar_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme);
    void render_logistics_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme);
    void render_customizer_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme);
    void render_projection_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme);
    void render_footer_bar(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme);

    void spawn_combat_visuals(const SimulationEngine& sim);
    static std::string format_number(double n);
};

} // namespace omnisbelli

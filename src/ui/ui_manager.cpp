#include "ui/ui_manager.hpp"
#include "gfx/bitmap_font.hpp"
#include "gfx/rasterizer.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace omnisbelli {

UIManager::UIManager()
    : active_tab(ActiveScreenTab::TAB_TACTICAL),
      customizer_selected_index(0),
      customizer_editing_swarm(false),
      rng(42) {}

std::string UIManager::format_number(double n) {
    if (std::isnan(n) || std::isinf(n)) return "0";
    if (n <= 0.0) return "0";
    if (n >= 1e18) {
        std::ostringstream ss;
        ss << std::scientific << std::setprecision(2) << n;
        return ss.str();
    }
    uint64_t val = static_cast<uint64_t>(std::round(n));
    std::string s = std::to_string(val);
    int insert_pos = static_cast<int>(s.length()) - 3;
    while (insert_pos > 0) {
        s.insert(insert_pos, ",");
        insert_pos -= 3;
    }
    return s;
}

void UIManager::switch_tab(ActiveScreenTab tab) {
    active_tab = tab;
}

void UIManager::cycle_tab() {
    int next = (static_cast<int>(active_tab) + 1) % 5;
    active_tab = static_cast<ActiveScreenTab>(next);
}

void UIManager::customizer_up() {
    if (customizer_selected_index > 0) {
        customizer_selected_index--;
    }
}

void UIManager::customizer_down(const SimulationEngine& sim) {
    const auto& roster = customizer_editing_swarm ? sim.attackers : sim.defenders;
    if (customizer_selected_index + 1 < static_cast<int>(roster.units.size())) {
        customizer_selected_index++;
    }
}

void UIManager::customizer_adjust(SimulationEngine& sim, double factor) {
    auto& roster = customizer_editing_swarm ? sim.attackers : sim.defenders;
    if (customizer_selected_index >= 0 && customizer_selected_index < static_cast<int>(roster.units.size())) {
        auto& u = roster.units[customizer_selected_index];
        double new_cnt = std::max(0.0, std::round(u.initial_count * factor));
        if (factor > 1.0 && new_cnt == u.initial_count) new_cnt += 100.0;
        u.initial_count = new_cnt;
        u.count = new_cnt;

        sim.synapse.update(sim.attackers, 0.0);
        sim.run_planetary_projection(20);
    }
}

void UIManager::customizer_toggle_side() {
    customizer_editing_swarm = !customizer_editing_swarm;
    customizer_selected_index = 0;
}

void UIManager::update_visuals(double dt, const SimulationEngine& sim) {
    if (dt <= 0.0) return;

    // Update active combat particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->x += it->vx * static_cast<float>(dt);
        it->y += it->vy * static_cast<float>(dt);
        it->life -= static_cast<float>(dt);
        if (it->life <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }

    if (!sim.is_paused && sim.outcome == SimulationOutcome::IN_PROGRESS) {
        spawn_combat_visuals(sim);
    }
}

void UIManager::spawn_combat_visuals(const SimulationEngine& sim) {
    if (particles.size() > 250) return;

    std::uniform_real_distribution<float> rand_flt(0.0f, 1.0f);
    int active_sec_idx = static_cast<int>(sim.sector_graph.active_sector_index);
    float sector_clash_x = 180.0f + active_sec_idx * 55.0f;

    // Friendly lasgun / bolter gunfire tracers (left to right)
    if (rand_flt(rng) < 0.65f) {
        VisualParticle p;
        p.x = sector_clash_x - 30.0f + rand_flt(rng) * 20.0f;
        p.y = 80.0f + rand_flt(rng) * 160.0f;
        p.vx = 120.0f + rand_flt(rng) * 80.0f;
        p.vy = (rand_flt(rng) - 0.5f) * 30.0f;
        p.life = 0.25f + rand_flt(rng) * 0.25f;
        p.color = crt.get_current_theme().friendly_unit;
        p.size = 2;
        particles.push_back(p);
    }

    // Heavy artillery explosions
    if (sim.last_telemetry.defender_dps_effective > 10000.0 && rand_flt(rng) < 0.25f) {
        VisualParticle p;
        p.x = sector_clash_x + 20.0f + rand_flt(rng) * 80.0f;
        p.y = 80.0f + rand_flt(rng) * 160.0f;
        p.vx = (rand_flt(rng) - 0.5f) * 15.0f;
        p.vy = (rand_flt(rng) - 0.5f) * 15.0f;
        p.life = 0.45f;
        p.color = crt.get_current_theme().alert_amber;
        p.size = 4;
        particles.push_back(p);
    }

    // Swarm bio-spores & advancing bio-forms (right to left)
    if (rand_flt(rng) < 0.70f) {
        VisualParticle p;
        p.x = sector_clash_x + 70.0f + rand_flt(rng) * 50.0f;
        p.y = 80.0f + rand_flt(rng) * 160.0f;
        p.vx = -(60.0f + rand_flt(rng) * 50.0f);
        p.vy = (rand_flt(rng) - 0.5f) * 40.0f;
        p.life = 0.4f + rand_flt(rng) * 0.3f;
        p.color = crt.get_current_theme().hostile_unit;
        p.size = 1;
        particles.push_back(p);
    }
}

void UIManager::render(PixelFramebuffer& fb, const SimulationEngine& sim) {
    ThemeColors theme = crt.get_current_theme();
    fb.clear(theme.bg_screen);

    render_header_bar(fb, sim, theme);

    switch (active_tab) {
    case ActiveScreenTab::TAB_TACTICAL:
        render_tactical_screen(fb, sim, theme);
        break;
    case ActiveScreenTab::TAB_RADAR:
        render_radar_screen(fb, sim, theme);
        break;
    case ActiveScreenTab::TAB_LOGISTICS:
        render_logistics_screen(fb, sim, theme);
        break;
    case ActiveScreenTab::TAB_CUSTOMIZER:
        render_customizer_screen(fb, sim, theme);
        break;
    case ActiveScreenTab::TAB_PROJECTION:
        render_projection_screen(fb, sim, theme);
        break;
    }

    render_footer_bar(fb, sim, theme);

    // Scanlines & phosphor post-processing
    crt.post_process(fb);
}

void UIManager::render_header_bar(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme) {
    Rasterizer::fill_rect(fb, 0, 0, 640, 24, theme.bg_panel);
    Rasterizer::draw_hline(fb, 0, 24, 640, theme.border);

    // Cogitator Glyph & Title (x: 4..182)
    BitmapFont::draw_glyph(fb, 4, 4, TacticalGlyph::COG, theme.alert_amber, 2);
    BitmapFont::draw_string(fb, 22, 4, "OMNISBELLI", theme.text_highlight, 0, true, 2);

    // Time & Speed / Status (x: 188..410)
    std::string time_str = sim.get_formatted_sim_time();
    BitmapFont::draw_string(fb, 188, 4, time_str, theme.text_primary, 0, true, 1);

    std::ostringstream spd;
    spd << "[" << (sim.is_paused ? "PAUSED" : "RUNNING") << " " << std::fixed << std::setprecision(0) << sim.time_multiplier << "X] // TARSUS";
    BitmapFont::draw_string(fb, 188, 14, spd.str(), sim.is_paused ? theme.alert_amber : theme.text_highlight, 0, true, 1);

    // 5 Clean Tab Buttons (x: 420..636)
    const char* tabs[5] = {"[1]TAC", "[2]RAD", "[3]LOG", "[4]ROS", "[5]PRJ"};
    int tab_x = 420;
    for (int i = 0; i < 5; i++) {
        bool active = (static_cast<int>(active_tab) == i);
        uint32_t col = active ? theme.text_highlight : theme.text_dim;
        uint32_t bg = active ? theme.border : theme.bg_panel;
        Rasterizer::fill_rect(fb, tab_x, 2, 40, 20, bg);
        Rasterizer::draw_rect(fb, tab_x, 2, 40, 20, active ? theme.text_highlight : theme.border);
        BitmapFont::draw_string(fb, tab_x + 3, 8, tabs[i], col, 0, true, 1);
        tab_x += 44;
    }
}

void UIManager::render_tactical_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme) {
    // 1. Left Panel: Sector Defense Hierarchy (x: 4, y: 28, w: 312, h: 215)
    Rasterizer::draw_frame(fb, 4, 28, 312, 215, theme.border, theme.bg_panel, true);
    BitmapFont::draw_string(fb, 8, 32, "[PLANETARY DEFENSE SECTOR HIERARCHY]", theme.text_highlight, 0, true, 1);

    int sec_y = 48;
    for (size_t i = 0; i < sim.sector_graph.sectors.size(); i++) {
        const auto& sec = sim.sector_graph.sectors[i];
        bool is_active = (i == sim.sector_graph.active_sector_index);

        // Status prefix
        std::string status_box = "[ ]";
        uint32_t status_col = theme.text_dim;
        if (sec.is_fallen()) {
            status_box = "[x]";
            status_col = theme.alert_red;
        } else if (is_active) {
            status_box = "[!]";
            status_col = (sec.breach_percentage >= 70.0) ? theme.alert_red : theme.alert_amber;
        }

        if (is_active) {
            Rasterizer::draw_rect(fb, 8, sec_y - 2, 302, 27, theme.border);
        }

        std::string sec_line = status_box + " [" + std::to_string(i + 1) + "] " + sec.name;
        BitmapFont::draw_string(fb, 10, sec_y, sec_line, is_active ? theme.text_highlight : status_col, 0, true, 1);

        // Breach & Chokepoint specs
        if (sec.is_fallen()) {
            BitmapFont::draw_string(fb, 36, sec_y + 10, "OVERRUN // DEFENDERS RETREATED", theme.alert_red, 0, true, 1);
        } else {
            // Breach bar
            double breach_ratio = sec.breach_percentage / 100.0;
            Rasterizer::draw_progress_bar(fb, 36, sec_y + 10, 80, 7, breach_ratio, theme.alert_red, theme.bar_empty, theme.border, false);

            std::ostringstream breach_txt;
            breach_txt << "BREACH:" << static_cast<int>(sec.breach_percentage) << "%";
            BitmapFont::draw_string(fb, 120, sec_y + 10, breach_txt.str(), (sec.breach_percentage > 50.0) ? theme.alert_red : theme.text_dim, 0, true, 1);

            std::ostringstream choke_txt;
            double choke_r = sec.get_chokepoint_ratio(sim.attackers.get_total_frontline_footprint());
            choke_txt << "CHOKE:" << std::fixed << std::setprecision(2) << choke_r << "x";
            BitmapFont::draw_string(fb, 205, sec_y + 10, choke_txt.str(), theme.text_primary, 0, true, 1);
        }

        sec_y += 31;
    }

    // 2. Right Panel: Tactical Casualty Matrix (x: 320, y: 28, w: 316, h: 215)
    Rasterizer::draw_frame(fb, 320, 28, 316, 215, theme.border, theme.bg_panel, true);
    BitmapFont::draw_string(fb, 324, 32, "[TACTICAL CASUALTY TELEMETRY]", theme.text_highlight, 0, true, 1);

    // Defender stats
    BitmapFont::draw_glyph(fb, 324, 46, TacticalGlyph::AQUILA, theme.friendly_unit, 1);
    std::string def_header = sim.defenders.name.substr(0, 24) + ":";
    BitmapFont::draw_string(fb, 336, 46, def_header, theme.text_highlight, 0, true, 1);

    double human_init = sim.defenders.get_total_initial_count();
    double human_curr = sim.defenders.get_total_count();
    double human_dead = sim.defenders.get_total_casualties();

    std::string hum_act = "  Active: " + format_number(human_curr) + " / " + format_number(human_init);
    BitmapFont::draw_string(fb, 324, 56, hum_act, theme.text_primary, 0, true, 1);

    double hum_dead_pct = (human_init > 0.0) ? (human_dead / human_init) * 100.0 : 0.0;
    std::ostringstream hum_slain;
    hum_slain << "  Losses: " << format_number(human_dead) << " (" << std::fixed << std::setprecision(1) << hum_dead_pct << "%)";
    BitmapFont::draw_string(fb, 324, 66, hum_slain.str(), (hum_dead_pct > 30.0) ? theme.alert_red : theme.text_dim, 0, true, 1);

    Rasterizer::draw_hline(fb, 324, 77, 308, theme.border);

    // Attacker stats
    BitmapFont::draw_glyph(fb, 324, 82, TacticalGlyph::BIOHAZARD, theme.hostile_unit, 1);
    std::string att_header = sim.attackers.name.substr(0, 24) + ":";
    BitmapFont::draw_string(fb, 336, 82, att_header, theme.alert_red, 0, true, 1);

    double swarm_init = sim.attackers.get_total_initial_count();
    double swarm_curr = sim.attackers.get_total_count();
    double swarm_dead = sim.attackers.get_total_casualties();

    std::string sw_act = "  Swarm : " + format_number(swarm_curr) + " / " + format_number(swarm_init);
    BitmapFont::draw_string(fb, 324, 92, sw_act, theme.hostile_unit, 0, true, 1);

    double sw_dead_pct = (swarm_init > 0.0) ? (swarm_dead / swarm_init) * 100.0 : 0.0;
    std::ostringstream sw_slain;
    sw_slain << "  Slain : " << format_number(swarm_dead) << " (" << std::fixed << std::setprecision(1) << sw_dead_pct << "%)";
    BitmapFont::draw_string(fb, 324, 102, sw_slain.str(), theme.text_dim, 0, true, 1);

    Rasterizer::draw_hline(fb, 324, 113, 308, theme.border);

    // Faction Signature Mechanics & Network Integrity
    BitmapFont::draw_glyph(fb, 324, 118, TacticalGlyph::SKULL, theme.synapse_glow, 1);
    double ratio = sim.attacker_rules ? sim.attacker_rules->get_telemetry_ratio() : sim.synapse.synapse_ratio;
    std::string tag = sim.attacker_rules ? sim.attacker_rules->get_telemetry_tag() : sim.synapse.get_state_tag();
    std::ostringstream syn_txt;
    syn_txt << "FACTION MATRIX: " << static_cast<int>(std::round(ratio * 100.0)) << "%";
    BitmapFont::draw_string(fb, 336, 118, syn_txt.str(), theme.text_highlight, 0, true, 1);

    Rasterizer::draw_progress_bar(fb, 324, 128, 140, 7, std::clamp(ratio, 0.0, 1.0), theme.synapse_glow, theme.bar_empty, theme.border, true);
    BitmapFont::draw_string(fb, 470, 128, tag.substr(0, 18), theme.text_dim, 0, true, 1);

    std::string desc = sim.attacker_rules ? sim.attacker_rules->get_status_description() : sim.synapse.get_status_description();
    BitmapFont::draw_string(fb, 324, 138, desc.substr(0, 48), theme.alert_amber, 0, true, 1);

    Rasterizer::draw_hline(fb, 324, 148, 308, theme.border);

    // Munitions & Power Grid Bars
    BitmapFont::draw_string(fb, 324, 153, "MUNITIONS & POWER STATUS:", theme.text_highlight, 0, true, 1);

    // Laspacks
    BitmapFont::draw_string(fb, 324, 163, "LASPACKS:", theme.text_dim, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 385, 163, 85, 6, sim.logistics.get_reserve_percentage(AmmoType::AMMO_LASPACKS) / 100.0, theme.friendly_unit, theme.bar_empty, theme.border, false);

    // Bolters
    BitmapFont::draw_string(fb, 476, 163, "BOLTERS:", theme.text_dim, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 530, 163, 85, 6, sim.logistics.get_reserve_percentage(AmmoType::AMMO_BOLTER_ROUNDS) / 100.0, theme.alert_amber, theme.bar_empty, theme.border, false);

    // Artillery
    BitmapFont::draw_string(fb, 324, 173, "SHELLS  :", theme.text_dim, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 385, 173, 85, 6, sim.logistics.get_reserve_percentage(AmmoType::AMMO_ARTILLERY_SHELLS) / 100.0, theme.bar_fill, theme.bar_empty, theme.border, false);

    // Promethium
    BitmapFont::draw_string(fb, 476, 173, "FUEL    :", theme.text_dim, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 530, 173, 85, 6, sim.logistics.get_reserve_percentage(AmmoType::AMMO_PROMETHIUM) / 100.0, theme.alert_red, theme.bar_empty, theme.border, false);

    // Titan Heat
    BitmapFont::draw_string(fb, 324, 183, "TITAN REACTOR HEAT:", theme.text_dim, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 470, 183, 145, 6, sim.logistics.titan_plasma_heat / 100.0, theme.alert_red, theme.bar_empty, theme.border, false);

    // Current Firepower Telemetry
    std::ostringstream dps_info;
    dps_info << "EFF DPS: DEF " << format_number(sim.last_telemetry.defender_dps_effective)
             << " vs ATT " << format_number(sim.last_telemetry.attacker_dps_effective);
    BitmapFont::draw_string(fb, 324, 202, dps_info.str(), theme.text_primary, 0, true, 1);

    std::ostringstream loss_info;
    loss_info << "TICK LOSS: -" << format_number(sim.last_telemetry.defender_casualties_step)
              << " Def / -" << format_number(sim.last_telemetry.attacker_casualties_step) << " Swarm";
    BitmapFont::draw_string(fb, 324, 214, loss_info.str(), theme.text_dim, 0, true, 1);

    // 3. Bottom Command Orders Panel (x: 4, y: 246, w: 632, h: 74)
    Rasterizer::draw_frame(fb, 4, 246, 632, 74, theme.border, theme.bg_panel, true);
    BitmapFont::draw_string(fb, 8, 250, "[TACTICAL COMMAND DOCTRINES: LORD GENERAL / MAGOS ORDERS]", theme.text_highlight, 0, true, 1);

    int doc_x = 8;
    int doc_y = 264;
    for (size_t i = 0; i < sim.orders.doctrines.size(); i++) {
        const auto& doc = sim.orders.doctrines[i];
        bool avail = doc.is_available;
        uint32_t btn_border = avail ? theme.border : theme.text_dim;
        uint32_t btn_text = avail ? theme.text_primary : theme.text_dim;

        Rasterizer::draw_rect(fb, doc_x, doc_y, 100, 24, btn_border);

        std::string key_lbl = "[" + doc.key_binding + "]";
        BitmapFont::draw_string(fb, doc_x + 3, doc_y + 3, key_lbl, avail ? theme.alert_amber : theme.text_dim, 0, true, 1);

        // Short label
        std::string short_name = doc.name.substr(0, 8);
        BitmapFont::draw_string(fb, doc_x + 30, doc_y + 3, short_name, btn_text, 0, true, 1);

        if (!avail && doc.cooldown_timer > 0.0) {
            std::ostringstream cd_s;
            cd_s << "CD: " << static_cast<int>(doc.cooldown_timer) << "s";
            BitmapFont::draw_string(fb, doc_x + 3, doc_y + 13, cd_s.str(), theme.alert_red, 0, true, 1);
        } else {
            BitmapFont::draw_string(fb, doc_x + 3, doc_y + 13, avail ? "READY" : "LOCKED", avail ? theme.text_highlight : theme.text_dim, 0, true, 1);
        }

        doc_x += 105;
        if (i == 2) {
            doc_x = 8;
            doc_y += 26;
        }
    }

    // Right side: Active Orders Dispatch & Telemetry (x: 328..632)
    Rasterizer::draw_vline(fb, 324, 250, 66, theme.border);
    BitmapFont::draw_string(fb, 330, 250, "[ORDERS DISPATCH & DOCTRINE STATUS]", theme.text_highlight, 0, true, 1);

    std::string action_sub = sim.orders.last_action_message;
    if (action_sub.length() > 37) action_sub = action_sub.substr(0, 37);
    BitmapFont::draw_string(fb, 330, 264, "> " + action_sub, theme.alert_amber, 0, true, 1);

    std::string l1 = (sim.orders.doctrines.size() > 1) ?
        ("F1: " + sim.orders.doctrines[0].name.substr(0, 14) + "  F2: " + sim.orders.doctrines[1].name.substr(0, 14)) : "F1/F2 Ready";
    std::string l2 = (sim.orders.doctrines.size() > 3) ?
        ("F3: " + sim.orders.doctrines[2].name.substr(0, 14) + "  F4: " + sim.orders.doctrines[3].name.substr(0, 14)) : "F3/F4 Ready";
    std::string l3 = (sim.orders.doctrines.size() > 5) ?
        ("F5: " + sim.orders.doctrines[4].name.substr(0, 14) + "  F8: " + sim.orders.doctrines[5].name.substr(0, 14)) : "F5/F8 Ready";
    BitmapFont::draw_string(fb, 330, 278, l1, theme.text_dim, 0, true, 1);
    BitmapFont::draw_string(fb, 330, 290, l2, theme.text_dim, 0, true, 1);
    BitmapFont::draw_string(fb, 330, 302, l3, theme.text_dim, 0, true, 1);
}

void UIManager::render_radar_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme) {
    Rasterizer::draw_frame(fb, 4, 28, 632, 292, theme.border, theme.bg_panel, true);
    BitmapFont::draw_string(fb, 8, 32, "[2D TACTICAL BATTLEFIELD RADAR // FORGE TARSUS TOPOGRAPHY]", theme.text_highlight, 0, true, 1);

    // Draw dithered radar grid
    for (int gx = 10; gx < 630; gx += 40) {
        Rasterizer::draw_vline(fb, gx, 46, 268, theme.bar_empty);
    }
    for (int gy = 46; gy < 314; gy += 40) {
        Rasterizer::draw_hline(fb, 10, gy, 620, theme.bar_empty);
    }

    // Defensive Sector Topography
    // 1. Orbital Defense perimeter arc
    Rasterizer::draw_line(fb, 50, 60, 50, 300, theme.text_dim);
    BitmapFont::draw_string(fb, 14, 52, "SEC-1", theme.text_dim, 0, true, 1);

    // 2. Ash Wastes barrier
    Rasterizer::draw_line(fb, 150, 60, 150, 300, theme.text_dim);
    BitmapFont::draw_string(fb, 114, 52, "SEC-2", theme.text_dim, 0, true, 1);

    // 3. Trench Line Alpha (zig zag trench)
    for (int ty = 60; ty < 300; ty += 20) {
        Rasterizer::draw_line(fb, 260, ty, 270, ty + 10, theme.border);
        Rasterizer::draw_line(fb, 270, ty + 10, 260, ty + 20, theme.border);
    }
    BitmapFont::draw_string(fb, 224, 52, "SEC-3 TRENCH", theme.text_dim, 0, true, 1);

    // 4. Bastion Iron Gates (thick fortress wall with narrow bottleneck opening)
    Rasterizer::fill_rect(fb, 380, 60, 12, 90, theme.text_highlight);
    Rasterizer::fill_rect(fb, 380, 210, 12, 90, theme.text_highlight);
    // Narrow blast gate
    Rasterizer::draw_rect(fb, 380, 150, 12, 60, theme.alert_amber);
    BitmapFont::draw_string(fb, 350, 52, "SEC-4 BASTION", theme.alert_amber, 0, true, 1);

    // 5. Hive Sub-Levels
    Rasterizer::draw_line(fb, 490, 60, 490, 300, theme.text_dim);
    BitmapFont::draw_string(fb, 460, 52, "SEC-5 HIVE", theme.text_dim, 0, true, 1);

    // 6. Spire Citadel (bunker)
    Rasterizer::draw_rect(fb, 570, 140, 45, 80, theme.text_highlight);
    BitmapFont::draw_glyph(fb, 584, 165, TacticalGlyph::AQUILA, theme.friendly_unit, 2);
    BitmapFont::draw_string(fb, 560, 52, "SEC-6 SPIRE", theme.text_highlight, 0, true, 1);

    // Highlight current active battle clash line
    int active_idx = static_cast<int>(sim.sector_graph.active_sector_index);
    int active_clash_x = 50 + active_idx * 105;
    Rasterizer::draw_vline(fb, active_clash_x, 46, 268, theme.alert_red);
    BitmapFont::draw_string(fb, active_clash_x - 30, 302, "[CLASH FRONT]", theme.alert_red, 0, true, 1);

    // Render Friendly Garrisons (icons on left of active sector)
    BitmapFont::draw_glyph(fb, active_clash_x - 20, 100, TacticalGlyph::SHIELD, theme.friendly_unit, 1);
    BitmapFont::draw_glyph(fb, active_clash_x - 20, 180, TacticalGlyph::CROSSHAIR, theme.friendly_unit, 1);
    BitmapFont::draw_glyph(fb, active_clash_x - 20, 240, TacticalGlyph::SHIELD, theme.friendly_unit, 1);

    // Render Swarm Biomass Clusters (hostile dots advancing from right)
    for (int sy = 70; sy < 290; sy += 18) {
        Rasterizer::fill_circle(fb, active_clash_x + 25 + (sy % 30), sy, 2, theme.hostile_unit);
        Rasterizer::fill_circle(fb, active_clash_x + 50 + ((sy * 3) % 25), sy + 5, 2, theme.hostile_unit);
        Rasterizer::fill_circle(fb, active_clash_x + 85 + (sy % 20), sy - 4, 3, theme.hostile_unit);
    }

    // Render animated combat particles (tracers, explosions, spore flights)
    for (const auto& p : particles) {
        int px = static_cast<int>(p.x);
        int py = static_cast<int>(p.y);
        if (p.size <= 1) {
            fb.set_pixel(px, py, p.color);
        } else if (p.size == 2) {
            Rasterizer::draw_line(fb, px, py, px - static_cast<int>(p.vx * 0.05f), py, p.color);
        } else {
            Rasterizer::fill_circle(fb, px, py, p.size, p.color);
        }
    }
}

void UIManager::render_logistics_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme) {
    Rasterizer::draw_frame(fb, 4, 28, 632, 292, theme.border, theme.bg_panel, true);
    BitmapFont::draw_string(fb, 8, 32, "[DEPARTMENTO MUNITORIUM // STRATEGIC LOGISTICS & SUPPLY GRID]", theme.text_highlight, 0, true, 1);

    int card_y = 50;

    auto render_supply_card = [&](
        const std::string& title,
        double cur, double max,
        const std::string& unit_label,
        double burn_rate,
        uint32_t bar_color
    ) {
        Rasterizer::draw_rect(fb, 12, card_y, 616, 44, theme.border);
        BitmapFont::draw_string(fb, 20, card_y + 5, title, theme.text_highlight, 0, true, 1);

        double pct = (max > 0.0) ? (cur / max) : 0.0;
        Rasterizer::draw_progress_bar(fb, 20, card_y + 18, 300, 12, pct, bar_color, theme.bar_empty, theme.border, true);

        std::ostringstream status;
        status << format_number(cur) << " / " << format_number(max) << " " << unit_label
               << " (" << static_cast<int>(std::round(pct * 100.0)) << "%)";
        BitmapFont::draw_string(fb, 330, card_y + 10, status.str(), theme.text_primary, 0, true, 1);

        std::ostringstream burn;
        burn << "CONSUMPTION RATE: " << format_number(burn_rate) << " " << unit_label << "/sec";
        BitmapFont::draw_string(fb, 330, card_y + 24, burn.str(), (cur <= 0.0) ? theme.alert_red : theme.text_dim, 0, true, 1);

        card_y += 50;
    };

    render_supply_card(
        "1. LASPACK POWER CELLS & GEOTHERMAL ENERGY GRID",
        sim.logistics.laspack_mwh, sim.logistics.laspack_max_mwh,
        "MWh", 120.0, theme.friendly_unit
    );

    render_supply_card(
        "2. HEAVY BOLTER MUNITIONS & ASTARTES ROUNDS",
        sim.logistics.bolter_rounds, sim.logistics.bolter_max_rounds,
        "Rounds", 450.0, theme.alert_amber
    );

    render_supply_card(
        "3. BASILISK & MEDUSA HEAVY ARTILLERY SHELLS",
        sim.logistics.artillery_shells, sim.logistics.artillery_max_shells,
        "Shells", 85.0, theme.bar_fill
    );

    render_supply_card(
        "4. REFINED PROMETHIUM FUEL (FLAME WEAPONS & DEFENSE MOATS)",
        sim.logistics.promethium_liters, sim.logistics.promethium_max_liters,
        "Liters", 0.0, theme.alert_red
    );

    render_supply_card(
        "5. TITAN GOD-ENGINE PLASMA REACTOR HEAT CAPACITY",
        100.0 - sim.logistics.titan_plasma_heat, 100.0,
        "% Margin", 1.5, theme.alert_red
    );
}

void UIManager::render_customizer_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme) {
    Rasterizer::draw_frame(fb, 4, 28, 632, 292, theme.border, theme.bg_panel, true);
    BitmapFont::draw_string(fb, 8, 32, "[TACTICAL ROSTER CUSTOMIZER // USER-PLUGGABLE MACRO UNIT FORMATIONS]", theme.text_highlight, 0, true, 1);

    BitmapFont::draw_string(
        fb, 8, 44,
        "CONTROLS: [UP/DN] Select [L/R] Scale [F] Def Faction [G] Att Faction [TAB] Side [L/6-0] Preset",
        theme.alert_amber, 0, true, 1
    );

    // Left column: Defender Units
    Rasterizer::draw_rect(fb, 10, 58, 305, 230, !customizer_editing_swarm ? theme.text_highlight : theme.border);
    std::string def_box_title = "DEFENDERS: " + sim.defenders.name.substr(0, 20);
    BitmapFont::draw_string(fb, 16, 62, def_box_title, !customizer_editing_swarm ? theme.text_highlight : theme.text_dim, 0, true, 1);

    int row_y = 76;
    for (size_t i = 0; i < sim.defenders.units.size(); i++) {
        const auto& u = sim.defenders.units[i];
        bool selected = (!customizer_editing_swarm && customizer_selected_index == static_cast<int>(i));

        if (selected) {
            Rasterizer::fill_rect(fb, 12, row_y - 2, 301, 19, theme.border);
        }

        std::string name = u.name.substr(0, 18);
        BitmapFont::draw_string(fb, 14, row_y, name, selected ? theme.text_highlight : theme.text_primary, 0, true, 1);

        std::string cnt_str = format_number(u.initial_count);
        BitmapFont::draw_string(fb, 180, row_y, cnt_str, selected ? theme.alert_amber : theme.text_dim, 0, true, 1);

        row_y += 21;
    }

    // Right column: Attacker Units
    Rasterizer::draw_rect(fb, 325, 58, 305, 230, customizer_editing_swarm ? theme.text_highlight : theme.border);
    std::string att_box_title = "ATTACKERS: " + sim.attackers.name.substr(0, 20);
    BitmapFont::draw_string(fb, 331, 62, att_box_title, customizer_editing_swarm ? theme.text_highlight : theme.text_dim, 0, true, 1);

    row_y = 76;
    for (size_t i = 0; i < sim.attackers.units.size(); i++) {
        const auto& u = sim.attackers.units[i];
        bool selected = (customizer_editing_swarm && customizer_selected_index == static_cast<int>(i));

        if (selected) {
            Rasterizer::fill_rect(fb, 327, row_y - 2, 301, 19, theme.border);
        }

        std::string name = u.name.substr(0, 18);
        BitmapFont::draw_string(fb, 329, row_y, name, selected ? theme.text_highlight : theme.hostile_unit, 0, true, 1);

        std::string cnt_str = format_number(u.initial_count);
        BitmapFont::draw_string(fb, 495, row_y, cnt_str, selected ? theme.alert_amber : theme.text_dim, 0, true, 1);

        row_y += 21;
    }

    // Totals comparison bar at bottom
    Rasterizer::fill_rect(fb, 10, 292, 620, 24, theme.bg_panel);
    std::string totals_msg = "TOTALS: " + format_number(sim.defenders.get_total_initial_count()) +
        " DEFENDERS vs " + format_number(sim.attackers.get_total_initial_count()) + " ATTACKERS";
    BitmapFont::draw_string(fb, 16, 298, totals_msg, theme.text_highlight, 0, true, 1);
}

void UIManager::render_projection_screen(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme) {
    Rasterizer::draw_frame(fb, 4, 28, 632, 292, theme.border, theme.bg_panel, true);
    BitmapFont::draw_string(fb, 8, 32, "[PLANETARY MONTE CARLO PROJECTION // LANCHESTER ATTRITION FORECAST]", theme.text_highlight, 0, true, 1);

    BitmapFont::draw_string(
        fb, 8, 44,
        "COMPUTED ACROSS MULTIPLE DIFFERENTIAL ENGAGEMENT RUNS. PRESS [M] TO RECOMPUTE.",
        theme.text_dim, 0, true, 1
    );

    const auto& proj = sim.cached_projection;

    // 1. Victory Probability Bar
    BitmapFont::draw_string(fb, 16, 62, "DEFENSIVE VICTORY PROBABILITY:", theme.friendly_unit, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 16, 74, 450, 14, proj.win_percentage / 100.0, theme.friendly_unit, theme.bar_empty, theme.border, true);
    std::ostringstream win_pct;
    win_pct << static_cast<int>(std::round(proj.win_percentage)) << "%";
    BitmapFont::draw_string(fb, 475, 76, win_pct.str(), theme.friendly_unit, 0, true, 1);

    // 2. Stalemate Probability Bar
    BitmapFont::draw_string(fb, 16, 96, "PERMANENT ATTRITION STALEMATE:", theme.alert_amber, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 16, 108, 450, 14, proj.stalemate_percentage / 100.0, theme.alert_amber, theme.bar_empty, theme.border, true);
    std::ostringstream stale_pct;
    stale_pct << static_cast<int>(std::round(proj.stalemate_percentage)) << "%";
    BitmapFont::draw_string(fb, 475, 110, stale_pct.str(), theme.alert_amber, 0, true, 1);

    // 3. Defeat / Extinction Probability Bar
    BitmapFont::draw_string(fb, 16, 130, "PLANETARY FALL & EXTINCTION:", theme.alert_red, 0, true, 1);
    Rasterizer::draw_progress_bar(fb, 16, 142, 450, 14, proj.defeat_percentage / 100.0, theme.alert_red, theme.bar_empty, theme.border, true);
    std::ostringstream def_pct;
    def_pct << static_cast<int>(std::round(proj.defeat_percentage)) << "%";
    BitmapFont::draw_string(fb, 475, 144, def_pct.str(), theme.alert_red, 0, true, 1);

    Rasterizer::draw_hline(fb, 16, 166, 608, theme.border);

    // Statistical Outcomes Table
    std::ostringstream days_s;
    days_s << "EXPECTED DAYS SURVIVED : " << std::fixed << std::setprecision(1) << proj.expected_days_survived << " Terran Days";
    BitmapFont::draw_string(fb, 16, 175, days_s.str(), theme.text_primary, 0, true, 1);

    std::ostringstream ammo_s;
    ammo_s << "CRITICAL MUNITIONS DRY : Day " << std::fixed << std::setprecision(1) << (proj.projected_ammo_depletion_hour / 24.0);
    BitmapFont::draw_string(fb, 16, 190, ammo_s.str(), theme.alert_amber, 0, true, 1);

    std::string hum_losses = "PROJECTED HUMAN CASUALTIES: " + format_number(proj.projected_human_casualties);
    BitmapFont::draw_string(fb, 16, 205, hum_losses, theme.text_primary, 0, true, 1);

    std::string swarm_losses = "PROJECTED SWARM CASUALTIES: " + format_number(proj.projected_swarm_casualties);
    BitmapFont::draw_string(fb, 16, 220, swarm_losses, theme.hostile_unit, 0, true, 1);

    Rasterizer::draw_hline(fb, 16, 238, 608, theme.border);

    // Strategic Assessment Dispatch
    BitmapFont::draw_string(fb, 16, 246, "HIGH COMMAND STRATEGIC ASSESSMENT:", theme.text_highlight, 0, true, 1);
    BitmapFont::draw_string(fb, 16, 260, proj.strategic_assessment, theme.alert_amber, 0, true, 1);

    std::string iter_str = "CALCULATION PASSES: " + std::to_string(proj.iterations_run) + " RUNS // 60 TPS INTEGRATION";
    BitmapFont::draw_string(fb, 16, 280, iter_str, theme.text_dim, 0, true, 1);
}

void UIManager::render_footer_bar(PixelFramebuffer& fb, const SimulationEngine& sim, const ThemeColors& theme) {
    Rasterizer::fill_rect(fb, 0, 322, 640, 38, theme.bg_panel);
    Rasterizer::draw_hline(fb, 0, 322, 640, theme.border);

    // Rolling Dispatch Ticker
    std::string latest_event = sim.event_log.empty() ? "> COGITATOR DISPATCH: ALL STATIONS REPORTING GREEN." : sim.event_log.back();
    BitmapFont::draw_string(fb, 6, 326, latest_event, theme.alert_amber, 0, true, 1);

    // Keyboard Shortcuts Banner
    std::string shortcuts = "[SPACE] Pause  [P] Palette  [C] CRT  [+/-] Speed  [M] MonteCarlo  [1-5] Tabs  [R] Reset";
    BitmapFont::draw_string(fb, 6, 340, shortcuts, theme.text_dim, 0, true, 1);
}

} // namespace omnisbelli

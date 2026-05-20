#pragma once
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"

namespace omnisbelli {

class EnvironmentalHazards {
public:
    double spore_toxicity;             // 0.0 to 100.0%
    bool subterranean_breach_active;
    double subterranean_timer;
    double commissar_executions_count;
    double spore_mines_active;
    double orbital_laser_cooldown;

    EnvironmentalHazards() 
        : spore_toxicity(0.0), subterranean_breach_active(false),
          subterranean_timer(0.0), commissar_executions_count(0.0),
          spore_mines_active(0.0), orbital_laser_cooldown(0.0) {}

    void update(double dt, ArmyRoster& def, ArmyRoster& att, Sector& active_sector) {
        // Atmospheric spore toxicity passive drain on unsealed infantry
        if (spore_toxicity > 20.0) {
            double drain_pct = (spore_toxicity / 100.0) * 0.002 * dt;
            for (auto& u : def.units) {
                // Space Marines, Custodes, Titans, and enclosed vehicles have sealed environmental systems
                if (u.id.find("marine") == std::string::npos && 
                    u.id.find("astartes") == std::string::npos &&
                    u.id.find("custodes") == std::string::npos && 
                    u.id.find("custodian") == std::string::npos &&
                    u.id.find("titan") == std::string::npos &&
                    u.id.find("leman") == std::string::npos &&
                    u.id.find("basilisk") == std::string::npos &&
                    u.id.find("tank") == std::string::npos) {
                    u.apply_direct_casualties(u.count * drain_pct);
                }
            }
        }

        // Subterranean Mawloc breach logic
        if (subterranean_breach_active) {
            subterranean_timer -= dt;
            if (subterranean_timer <= 0.0) {
                subterranean_breach_active = false;
            } else {
                // Direct backline strike bypassing walls
                for (auto& u : def.units) {
                    if (u.backline_ratio > 0.5 && u.count > 0.0) {
                        u.apply_direct_casualties(u.count * 0.005 * dt);
                    }
                }
            }
        }

        // Laser cooldown
        if (orbital_laser_cooldown > 0.0) {
            orbital_laser_cooldown = std::max(0.0, orbital_laser_cooldown - dt);
        }
    }

    bool trigger_commissar_execution(ArmyRoster& def) {
        // Executes 1 trooper to instantly restore unit resolve
        for (auto& u : def.units) {
            if (u.id.find("infantry") != std::string::npos && u.count > 10.0) {
                u.apply_direct_casualties(1.0);
                commissar_executions_count += 1.0;
                return true;
            }
        }
        return false;
    }

    void trigger_subterranean_breach(double duration = 15.0) {
        subterranean_breach_active = true;
        subterranean_timer = duration;
    }

    double trigger_orbital_defense_laser(ArmyRoster& att) {
        if (orbital_laser_cooldown > 0.0) return 0.0;
        orbital_laser_cooldown = 45.0;
        double total_kills = 0.0;
        for (auto& u : att.units) {
            if (u.backline_ratio <= 0.5) {
                double kills = u.apply_direct_casualties(u.count * 0.08); // Vaporizes 8% of frontal wave
                total_kills += kills;
            }
        }
        return total_kills;
    }

    void add_spore_toxicity(double delta) {
        spore_toxicity = std::min(100.0, std::max(0.0, spore_toxicity + delta));
    }
};

} // namespace omnisbelli

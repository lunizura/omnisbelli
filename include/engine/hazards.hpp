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

    void add_spore_toxicity(double delta) {
        spore_toxicity = std::min(100.0, std::max(0.0, spore_toxicity + delta));
    }
};

} // namespace omnisbelli

#pragma once
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include "engine/logistics.hpp"
#include <string>

namespace omnisbelli {

struct PlanetaryProjection {
    double win_percentage;
    double stalemate_percentage;
    double defeat_percentage;
    double expected_days_survived;
    double projected_human_casualties;
    double projected_swarm_casualties;
    double projected_ammo_depletion_hour;
    std::string strategic_assessment;
    int iterations_run;
};

class ProjectionEngine {
public:
    static PlanetaryProjection run_monte_carlo(
        const ArmyRoster& defenders_template,
        const ArmyRoster& attackers_template,
        const SectorGraph& sector_graph_template,
        const LogisticsSystem& logistics_template,
        int iterations = 40
    );
};

} // namespace omnisbelli

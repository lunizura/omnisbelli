#pragma once
#include "engine/army_roster.hpp"
#include "engine/logistics.hpp"
#include "engine/synapse_network.hpp"
#include "engine/sector.hpp"

namespace omnisbelli {

struct CombatStepTelemetry {
    double defender_dps_raw;
    double defender_dps_effective;
    double attacker_dps_raw;
    double attacker_dps_effective;
    double chokepoint_ratio;
    double synapse_ratio;
    double defender_casualties_step;
    double attacker_casualties_step;
    double sector_breach_step;
};

class LanchesterCombat {
public:
    static void compute_combat_tick(
        ArmyRoster& defenders,
        ArmyRoster& attackers,
        Sector& active_sector,
        LogisticsSystem& logistics,
        SynapseNetwork& synapse,
        double dt,
        CombatStepTelemetry& telemetry
    );
};

} // namespace omnisbelli

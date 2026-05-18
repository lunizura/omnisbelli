#pragma once
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include "engine/logistics.hpp"
#include "engine/synapse_network.hpp"
#include "engine/command_orders.hpp"
#include "engine/lanchester.hpp"
#include "engine/projection.hpp"
#include <string>
#include <vector>

namespace omnisbelli {

enum class SimulationOutcome {
    IN_PROGRESS,
    IMPERIAL_VICTORY,
    TYRANID_VICTORY,
    EXTERMINATUS_PURGE,
    STALEMATE_ATTRITION
};

class SimulationEngine {
public:
    ArmyRoster defenders;
    ArmyRoster attackers;
    SectorGraph sector_graph;
    LogisticsSystem logistics;
    SynapseNetwork synapse;
    CommandOrders orders;

    CombatStepTelemetry last_telemetry;
    PlanetaryProjection cached_projection;

    double sim_time_seconds;
    double time_multiplier;
    bool is_paused;
    SimulationOutcome outcome;
    std::string outcome_banner;

    std::vector<std::string> event_log;

    SimulationEngine();

    void update(double real_dt);
    void add_event(const std::string& msg);

    void toggle_pause();
    void set_speed(double multiplier);
    void cycle_speed();

    bool trigger_doctrine(DoctrineType type);
    void run_planetary_projection(int iterations = 30);

    void reset();
    void load_scenario(int preset_index); // 0: Tyran 2M vs 100M, 1: Macragge Polar, 2: Krieg Trench, 3: Armageddon Orks, 4: Necron Awakening
    std::string get_formatted_sim_time() const;
};

} // namespace omnisbelli

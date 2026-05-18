#include "engine/simulation.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace omnisbelli {

SimulationEngine::SimulationEngine()
    : sim_time_seconds(0.0),
      time_multiplier(1.0),
      is_paused(false),
      outcome(SimulationOutcome::IN_PROGRESS),
      outcome_banner("DEFENSE IN PROGRESS") {

    defenders = ArmyRoster::create_imperial_defense(1.0);
    attackers = ArmyRoster::create_tyranid_swarm(1.0);
    sector_graph = SectorGraph::create_standard_planetary_graph();
    logistics = LogisticsSystem();
    synapse = SynapseNetwork();
    orders = CommandOrders();

    last_telemetry = CombatStepTelemetry{0, 0, 0, 0, 1.0, 1.0, 0, 0, 0};

    add_event("[SYSTEM INITIALIZED] FORGE WORLD TARSUS DEFENSE COGITATOR ONLINE.");
    add_event("[DEPLOYMENT] 1,978,020 IMPERIAL DEFENDERS ENTRENCHED ACROSS 6 SECTORS.");
    add_event("[INVASION] 100,000,020 TYRANID BIO-FORMS ENTERING PLANETARY ATMOSPHERE.");

    run_planetary_projection(20);
}

void SimulationEngine::update(double real_dt) {
    if (is_paused || outcome != SimulationOutcome::IN_PROGRESS) {
        return;
    }

    double total_step = real_dt * time_multiplier;
    // Sub-stepping to maintain numerical stability during high-speed simulation
    const double max_substep = 0.1;
    while (total_step > 0.00001) {
        double sub_dt = std::min(max_substep, total_step);
        total_step -= sub_dt;
        sim_time_seconds += sub_dt;

        orders.update(sub_dt, sector_graph);

        Sector& cur_sec = sector_graph.get_active_sector();
        bool was_fallen = cur_sec.is_fallen();

        LanchesterCombat::compute_combat_tick(
            defenders,
            attackers,
            cur_sec,
            logistics,
            synapse,
            sub_dt,
            last_telemetry
        );

        // Check if sector fell this sub-step
        if (!was_fallen && cur_sec.is_fallen()) {
            add_event("[CRITICAL BREACH] " + cur_sec.name + " HAS FALLEN!");
            if (sector_graph.advance_active_sector()) {
                add_event("[RETREAT] DEFENDERS RE-FORMING AT " + sector_graph.get_active_sector().name);
            } else {
                // Spire Citadel has fallen
                outcome = SimulationOutcome::TYRANID_VICTORY;
                outcome_banner = "[EXTINCTION] SPIRE CITADEL OVERRUN. FORGE WORLD CONSUMED.";
                add_event(outcome_banner);
                break;
            }
        }

        // Check instinctive frenzy transition
        if (synapse.is_in_instinctive_frenzy() && synapse.time_in_feral_state <= sub_dt) {
            add_event("[SYNAPSE COLLAPSE] SWARM CONTROL SEVERED! INSTINCTIVE FRENZY TRIGGERED!");
        }

        // Check victory / defeat
        if (attackers.is_defeated()) {
            outcome = SimulationOutcome::IMPERIAL_VICTORY;
            outcome_banner = "[GLORY] TYRANID SWARM EXTERMINATED! THE IMPERIUM STANDS!";
            add_event(outcome_banner);
            break;
        }

        if (defenders.is_defeated()) {
            outcome = SimulationOutcome::TYRANID_VICTORY;
            outcome_banner = "[DEFEAT] ALL DEFENDERS SLAIN. BIOMASS HARVEST IMMINENT.";
            add_event(outcome_banner);
            break;
        }

        if (orders.exterminatus_triggered) {
            outcome = SimulationOutcome::EXTERMINATUS_PURGE;
            outcome_banner = "[PURGED] EXTERMINATUS COMPLETED. ZERO BIOMASS SURVIVES.";
            break;
        }
    }
}

void SimulationEngine::add_event(const std::string& msg) {
    event_log.push_back(msg);
    if (event_log.size() > 80) {
        event_log.erase(event_log.begin());
    }
}

void SimulationEngine::toggle_pause() {
    is_paused = !is_paused;
    add_event(is_paused ? "[COGITATOR] SIMULATION PAUSED." : "[COGITATOR] SIMULATION RESUMED.");
}

void SimulationEngine::set_speed(double multiplier) {
    time_multiplier = std::clamp(multiplier, 0.1, 200.0);
    std::ostringstream ss;
    ss << "[COGITATOR] SPEED SET TO " << std::fixed << std::setprecision(1) << time_multiplier << "X";
    add_event(ss.str());
}

void SimulationEngine::cycle_speed() {
    if (time_multiplier < 2.0) {
        set_speed(5.0);
    } else if (time_multiplier < 10.0) {
        set_speed(20.0);
    } else if (time_multiplier < 50.0) {
        set_speed(100.0);
    } else {
        set_speed(1.0);
    }
}

bool SimulationEngine::trigger_doctrine(DoctrineType type) {
    bool ok = orders.trigger_doctrine(
        type,
        defenders,
        attackers,
        sector_graph,
        logistics,
        synapse
    );
    if (ok) {
        add_event(orders.last_action_message);
    } else if (!orders.last_action_message.empty()) {
        add_event(orders.last_action_message);
    }
    return ok;
}

void SimulationEngine::run_planetary_projection(int iterations) {
    cached_projection = ProjectionEngine::run_monte_carlo(
        defenders,
        attackers,
        sector_graph,
        logistics,
        iterations
    );
    add_event("[TELEMETRY] PLANETARY MONTE CARLO PROJECTION UPDATED (" +
              std::to_string(cached_projection.iterations_run) + " RUNS).");
}

void SimulationEngine::reset() {
    sim_time_seconds = 0.0;
    time_multiplier = 1.0;
    is_paused = false;
    outcome = SimulationOutcome::IN_PROGRESS;
    outcome_banner = "DEFENSE IN PROGRESS";

    defenders.reset();
    attackers.reset();
    sector_graph.reset();
    logistics.reset();
    synapse.reset();
    orders.reset();

    event_log.clear();
    add_event("[COGITATOR RESET] BATTLE SIMULATION REINITIALIZED.");
    run_planetary_projection(20);
}

void SimulationEngine::load_scenario(int preset_index) {
    reset();
    switch (preset_index) {
    case 0: // Standard: Fall of Tyran (2M vs 100M)
        defenders = ArmyRoster::create_imperial_defense(1.0);
        attackers = ArmyRoster::create_tyranid_swarm(1.0);
        add_event("[SCENARIO LOADED] 1. FALL OF TYRAN (2M IMPERIUM VS 100M TYRANID SWARM)");
        break;
    case 1: // Macragge Polar Fortress (4M vs 250M)
        defenders = ArmyRoster::create_imperial_defense(2.0);
        attackers = ArmyRoster::create_tyranid_swarm(2.5);
        add_event("[SCENARIO LOADED] 2. MACRAGGE POLAR FORTRESS (4M IMPERIUM VS 250M TYRANIDS)");
        break;
    case 2: // Death Korps Krieg 88th Siege (10M vs 50M)
        defenders = ArmyRoster::create_imperial_defense(5.0);
        attackers = ArmyRoster::create_tyranid_swarm(0.5);
        add_event("[SCENARIO LOADED] 3. KRIEG 88TH SIEGE ARMY (10M IMPERIUM VS 50M TYRANIDS)");
        break;
    case 3: // Third War for Armageddon (3M vs 100M Ork WAAAGH!)
        defenders = ArmyRoster::create_imperial_defense(1.5);
        attackers = ArmyRoster::create_ork_waaagh(2.0);
        add_event("[SCENARIO LOADED] 4. THIRD WAR FOR ARMAGEDDON (3M IMPERIUM VS 90M ORK WAAAGH!)");
        break;
    case 4: // Necron Tomb World Awakening (2M vs 35M Necrons)
        defenders = ArmyRoster::create_imperial_defense(1.0);
        attackers = ArmyRoster::create_necron_dynasty(2.0);
        add_event("[SCENARIO LOADED] 5. TOMB WORLD AWAKENING (2M IMPERIUM VS 35M NECRON LEGION)");
        break;
    default:
        break;
    }
    run_planetary_projection(25);
}

std::string SimulationEngine::get_formatted_sim_time() const {
    int total_secs = static_cast<int>(sim_time_seconds);
    int days = total_secs / 86400;
    int hours = (total_secs % 86400) / 3600;
    int minutes = (total_secs % 3600) / 60;
    int seconds = total_secs % 60;

    std::ostringstream ss;
    ss << "DAY " << std::setw(2) << std::setfill('0') << days
       << " // " << std::setw(2) << std::setfill('0') << hours
       << ":" << std::setw(2) << std::setfill('0') << minutes
       << ":" << std::setw(2) << std::setfill('0') << seconds;
    return ss.str();
}

} // namespace omnisbelli

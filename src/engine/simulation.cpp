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

    sync_rule_modules();

    last_telemetry = CombatStepTelemetry{0, 0, 0, 0, 1.0, 1.0, 0, 0, 0};

    add_event("[SYSTEM INITIALIZED] FORGE WORLD TARSUS DEFENSE COGITATOR ONLINE.");
    add_event("[DEPLOYMENT] 1,978,020 IMPERIAL DEFENDERS ENTRENCHED ACROSS 6 SECTORS.");
    add_event("[INVASION] 100,000,020 TYRANID BIO-FORMS ENTERING PLANETARY ATMOSPHERE.");

    run_planetary_projection(20);
}

void SimulationEngine::sync_rule_modules() {
    defender_rules = create_faction_rule_module(defenders.faction_id);
    attacker_rules = create_faction_rule_module(attackers.faction_id);
}

void SimulationEngine::set_defender_faction(const std::string& faction_id) {
    if (faction_id == "imperium") defenders = ArmyRoster::create_imperial_defense(1.0);
    else if (faction_id == "death_guard") defenders = ArmyRoster::create_death_guard(1.0);
    else if (faction_id == "sororitas" || faction_id == "adepta_sororitas") defenders = ArmyRoster::create_adepta_sororitas(1.0);
    else if (faction_id == "mechanicus" || faction_id == "adeptus_mechanicus") defenders = ArmyRoster::create_adeptus_mechanicus(1.0);
    else if (faction_id == "world_eaters" || faction_id == "khorne") defenders = ArmyRoster::create_world_eaters(1.0);
    else if (faction_id == "krieg") defenders = ArmyRoster::create_krieg_siege_army(1.0);
    else if (faction_id == "iron_warriors") defenders = ArmyRoster::create_iron_warriors(1.0);
    else if (faction_id == "custodes") defenders = ArmyRoster::create_custodes_vanguard(1.0);
    else if (faction_id == "tau") defenders = ArmyRoster::create_tau_empire(1.0);
    else if (faction_id == "aeldari") defenders = ArmyRoster::create_aeldari_host(1.0);
    else defenders = ArmyRoster::create_imperial_defense(1.0);

    defender_rules = create_faction_rule_module(defenders.faction_id);
    add_event("[CUSTOMIZER] DEFENDER FACTION SET TO: " + defenders.name);
    run_planetary_projection(20);
}

void SimulationEngine::set_attacker_faction(const std::string& faction_id) {
    if (faction_id == "tyranids") attackers = ArmyRoster::create_tyranid_swarm(1.0);
    else if (faction_id == "death_guard") attackers = ArmyRoster::create_death_guard(1.0);
    else if (faction_id == "sororitas" || faction_id == "adepta_sororitas") attackers = ArmyRoster::create_adepta_sororitas(1.0);
    else if (faction_id == "mechanicus" || faction_id == "adeptus_mechanicus") attackers = ArmyRoster::create_adeptus_mechanicus(1.0);
    else if (faction_id == "world_eaters" || faction_id == "khorne") attackers = ArmyRoster::create_world_eaters(1.0);
    else if (faction_id == "orks") attackers = ArmyRoster::create_ork_waaagh(1.0);
    else if (faction_id == "necrons") attackers = ArmyRoster::create_necron_dynasty(1.0);
    else if (faction_id == "iron_warriors") attackers = ArmyRoster::create_iron_warriors(1.0);
    else if (faction_id == "chaos") attackers = ArmyRoster::create_chaos_incursion(1.0);
    else attackers = ArmyRoster::create_tyranid_swarm(1.0);

    attacker_rules = create_faction_rule_module(attackers.faction_id);
    add_event("[CUSTOMIZER] ATTACKER FACTION SET TO: " + attackers.name);
    run_planetary_projection(20);
}

void SimulationEngine::update(double real_dt) {
    if (is_paused || outcome != SimulationOutcome::IN_PROGRESS) {
        return;
    }

    if (!defender_rules || !attacker_rules) {
        sync_rule_modules();
    }

    double total_step = real_dt * time_multiplier;
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
            *defender_rules,
            *attacker_rules,
            sub_dt,
            last_telemetry
        );

        auto* tyranid_module = dynamic_cast<TyranidSynapseModule*>(attacker_rules.get());
        if (tyranid_module) {
            synapse = tyranid_module->synapse;
            if (synapse.is_in_instinctive_frenzy() && synapse.time_in_feral_state <= sub_dt) {
                add_event("[SYNAPSE COLLAPSE] SWARM CONTROL SEVERED! INSTINCTIVE FRENZY TRIGGERED!");
            }
        }

        // Check if sector fell this sub-step
        if (!was_fallen && cur_sec.is_fallen()) {
            add_event("[CRITICAL BREACH] " + cur_sec.name + " HAS FALLEN!");
            if (sector_graph.advance_active_sector()) {
                add_event("[RETREAT] DEFENDERS RE-FORMING AT " + sector_graph.get_active_sector().name);
            } else {
                outcome = SimulationOutcome::TYRANID_VICTORY;
                outcome_banner = "[ANNIHILATION] SPIRE CITADEL OVERRUN. SECTOR CONQUERED.";
                add_event(outcome_banner);
                break;
            }
        }

        // Check victory / defeat
        if (attackers.is_defeated()) {
            outcome = SimulationOutcome::IMPERIAL_VICTORY;
            outcome_banner = "[VICTORY] ATTACKING HOST BROKEN! DEFENDERS HOLD THE WORLD!";
            add_event(outcome_banner);
            break;
        }

        if (defenders.is_defeated()) {
            outcome = SimulationOutcome::TYRANID_VICTORY;
            outcome_banner = "[DEFEAT] ALL DEFENDERS SLAIN. INVASION FORCE PREVAILS.";
            add_event(outcome_banner);
            break;
        }

        if (orders.exterminatus_triggered) {
            outcome = SimulationOutcome::EXTERMINATUS_PURGE;
            outcome_banner = "[PURGED] EXTERMINATUS COMPLETED. ZERO LIFE SIGNS REMAIN.";
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

    sync_rule_modules();

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
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 1. FALL OF TYRAN (2M IMPERIUM VS 100M TYRANID SWARM)");
        break;
    case 1: // Macragge Polar Fortress (4M vs 250M)
        defenders = ArmyRoster::create_imperial_defense(2.0);
        attackers = ArmyRoster::create_tyranid_swarm(2.5);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 2. MACRAGGE POLAR FORTRESS (4M IMPERIUM VS 250M TYRANIDS)");
        break;
    case 2: // Death Korps Krieg 88th Siege (10M vs 50M)
        defenders = ArmyRoster::create_krieg_siege_army(1.0);
        attackers = ArmyRoster::create_tyranid_swarm(0.5);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 3. KRIEG 88TH SIEGE ARMY (KRIEG VS TYRANIDS)");
        break;
    case 3: // Third War for Armageddon (3M vs 100M Ork WAAAGH!)
        defenders = ArmyRoster::create_imperial_defense(1.5);
        attackers = ArmyRoster::create_ork_waaagh(2.0);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 4. THIRD WAR FOR ARMAGEDDON (3M IMPERIUM VS 90M ORK WAAAGH!)");
        break;
    case 4: // Necron Tomb World Awakening (2M vs 35M Necrons)
        defenders = ArmyRoster::create_imperial_defense(1.0);
        attackers = ArmyRoster::create_necron_dynasty(2.0);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 5. TOMB WORLD AWAKENING (2M IMPERIUM VS 35M NECRON LEGION)");
        break;
    case 5: // Plague Wars of Ultramar (Imperium vs Death Guard)
        defenders = ArmyRoster::create_imperial_defense(1.5);
        attackers = ArmyRoster::create_death_guard(1.0);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 6. PLAGUE WARS OF ULTRAMAR (IMPERIUM VS DEATH GUARD VECTORIUM)");
        break;
    case 6: // Taghmata Mars Awakening (Mechanicus vs Necrons)
        defenders = ArmyRoster::create_adeptus_mechanicus(1.0);
        attackers = ArmyRoster::create_necron_dynasty(1.5);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 7. TAGHMATA AWAKENING (ADEPTUS MECHANICUS VS NECRONS)");
        break;
    case 7: // Sanctuarium Crusade (Adepta Sororitas vs World Eaters)
        defenders = ArmyRoster::create_adepta_sororitas(1.0);
        attackers = ArmyRoster::create_world_eaters(1.0);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 8. SANCTUARIUM CRUSADE (ADEPTA SORORITAS VS WORLD EATERS)");
        break;
    case 8: // Iron Cage Siege (Iron Warriors vs Imperial Defense)
        defenders = ArmyRoster::create_imperial_defense(2.0);
        attackers = ArmyRoster::create_iron_warriors(1.5);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 9. IRON CAGE SIEGE (IMPERIUM VS IRON WARRIORS)");
        break;
    case 9: // Khorne Blood Incursion (World Eaters vs Death Guard)
        defenders = ArmyRoster::create_death_guard(1.0);
        attackers = ArmyRoster::create_world_eaters(1.2);
        sync_rule_modules();
        add_event("[SCENARIO LOADED] 10. KHORNE BLOOD INCURSION (DEATH GUARD VS WORLD EATERS)");
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

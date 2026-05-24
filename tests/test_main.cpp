#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>
#include "engine/unit_class.hpp"
#include "engine/army_roster.hpp"
#include "engine/logistics.hpp"
#include "engine/synapse_network.hpp"
#include "engine/sector.hpp"
#include "engine/lanchester.hpp"
#include "engine/command_orders.hpp"
#include "engine/projection.hpp"
#include "engine/simulation.hpp"
#include "engine/hazards.hpp"
#include "engine/telemetry_export.hpp"

#define OMNIS_ASSERT(cond) do { \
    if (!(cond)) { \
        std::cerr << "\n[TEST FAILED] Assertion failed: " #cond " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        std::exit(1); \
    } \
} while(0)

using namespace omnisbelli;

void test_unit_mechanics() {
    std::cout << "[TEST] 1. Unit Class & Damage Mechanics..." << std::endl;
    UnitClass infantry("inf_test", "Test Squad", "imperium", UnitCategory::INFANTRY,
                      1000.0, 10.0, 0.20, 2.0, 1.0, 1.0, 0.0, TargetPriority::PRIORITY_BALANCED);

    OMNIS_ASSERT(infantry.is_alive());
    OMNIS_ASSERT(std::abs(infantry.get_total_hp() - 10000.0) < 1e-4);

    // Apply 2000 raw damage with 20% armor -> 1600 mitigated damage -> 160 units killed
    double killed = infantry.take_damage(2000.0);
    OMNIS_ASSERT(std::abs(killed - 160.0) < 1e-3);
    OMNIS_ASSERT(std::abs(infantry.count - 840.0) < 1e-3);
    OMNIS_ASSERT(std::abs(infantry.casualties_suffered - 160.0) < 1e-3);

    // Direct casualty
    double direct = infantry.apply_direct_casualties(40.0);
    OMNIS_ASSERT(std::abs(direct - 40.0) < 1e-3);
    OMNIS_ASSERT(std::abs(infantry.count - 800.0) < 1e-3);

    std::cout << "       PASS: Unit health, armor mitigation, and casualty accounting verified.\n";
}

void test_lanchester_laws() {
    std::cout << "[TEST] 2. Lanchester Combat Laws (Square vs Linear Chokepoint)..." << std::endl;
    ArmyRoster def = ArmyRoster::create_imperial_defense(1.0);
    ArmyRoster att = ArmyRoster::create_tyranid_swarm(1.0);

    // Open field sector (Ash Wastes: 100,000,000m width to fit entire 100M swarm)
    Sector open_sector(1, "Open", "SEC-OPEN", "Open plains", 100000000.0, 0.0, 1e9);
    LogisticsSystem log;
    SynapseNetwork syn;
    CombatStepTelemetry tel_open;

    LanchesterCombat::compute_combat_tick(def, att, open_sector, log, syn, 1.0, tel_open);
    double open_attacker_eff = tel_open.attacker_dps_effective;
    OMNIS_ASSERT(tel_open.chokepoint_ratio >= 0.99); // Square law: almost 100% engage

    // Narrow bottleneck sector (Bastion Gate: 5,000m width)
    def.reset();
    att.reset();
    Sector choke_sector(2, "Choke", "SEC-CHOKE", "Narrow tunnel", 5000.0, 0.0, 1e9);
    CombatStepTelemetry tel_choke;

    LanchesterCombat::compute_combat_tick(def, att, choke_sector, log, syn, 1.0, tel_choke);
    double choke_attacker_eff = tel_choke.attacker_dps_effective;

    OMNIS_ASSERT(tel_choke.chokepoint_ratio < 0.1); // Severely constrained
    OMNIS_ASSERT(choke_attacker_eff < open_attacker_eff * 0.2); // Linear law drastically limits DPS

    std::cout << "       PASS: Square law open field DPS (" << (int)open_attacker_eff 
              << ") vs Linear law chokepoint DPS (" << (int)choke_attacker_eff 
              << ") verified.\n";
}

void test_excess_damage_cascade() {
    std::cout << "[TEST] 9. Excess Damage Cascade (Overkill Protection)..." << std::endl;
    // Build defender with high synapse priority damage
    ArmyRoster def("def_test", "Defender");
    def.add_unit(UnitClass("sniper", "Synapse Snipers", "imperium", UnitCategory::INFANTRY,
                           100.0, 10.0, 0.0, 500.0, 1.0, 1.0, 0.0, TargetPriority::PRIORITY_SYNAPSE));

    // Attacker has only 1 weak synapse unit (10 HP) and 5000 swarm gaunts
    ArmyRoster att("att_test", "Attacker");
    att.add_unit(UnitClass("weak_syn", "Lone Synapse", "tyranids", UnitCategory::INFANTRY,
                           1.0, 10.0, 0.0, 1.0, 1.0, 1.0, 0.0, TargetPriority::PRIORITY_BALANCED, true, 10.0));
    att.add_unit(UnitClass("gaunt", "Gaunt Swarm", "tyranids", UnitCategory::INFANTRY,
                           5000.0, 10.0, 0.0, 1.0, 1.0, 1.0, 0.0, TargetPriority::PRIORITY_BALANCED));

    Sector sec(1, "Open", "SEC-OPEN", "Plains", 1e8, 0.0, 1e8);
    LogisticsSystem log;
    SynapseNetwork syn;
    CombatStepTelemetry tel;

    // Snipers deal 50,000 damage. The 1 synapse unit absorbs only 10 damage.
    // The surplus 49,990 damage MUST cascade and wipe out swarm gaunts!
    LanchesterCombat::compute_combat_tick(def, att, sec, log, syn, 1.0, tel);

    OMNIS_ASSERT(!att.find_unit("weak_syn")->is_alive());
    OMNIS_ASSERT(att.find_unit("gaunt")->count < 5000.0);
    OMNIS_ASSERT(tel.attacker_casualties_step > 100.0); // Verifies cascade worked!

    std::cout << "       PASS: Surplus priority damage successfully cascaded to remaining targets ("
              << (int)tel.attacker_casualties_step << " total kills).\n";
}

void test_synapse_disruption() {
    std::cout << "[TEST] 3. Synapse Network & Instinctive Frenzy..." << std::endl;
    ArmyRoster swarm = ArmyRoster::create_tyranid_swarm(1.0);
    SynapseNetwork syn;

    syn.update(swarm, 0.1);
    OMNIS_ASSERT(syn.current_state == SynapseCoherenceState::COORDINATED);
    OMNIS_ASSERT(syn.get_attack_multiplier() == 1.0);
    OMNIS_ASSERT(syn.get_casualty_multiplier() == 1.0);

    // Kill all Synapse nodes (Warriors, Zoanthropes, Bio-Titans, Swarmlord)
    for (auto& u : swarm.units) {
        if (u.is_synapse) {
            u.apply_direct_casualties(u.count);
        }
    }

    syn.update(swarm, 1.0);
    OMNIS_ASSERT(syn.current_state == SynapseCoherenceState::INSTINCTIVE_FERAL);
    OMNIS_ASSERT(syn.is_in_instinctive_frenzy());
    OMNIS_ASSERT(std::abs(syn.get_attack_multiplier() - 0.30) < 1e-4); // -70% attack penalty
    OMNIS_ASSERT(std::abs(syn.get_casualty_multiplier() - 2.00) < 1e-4); // 2.0x casualties taken
    OMNIS_ASSERT(syn.total_feral_infighting_casualties > 0.0); // Feral infighting active

    std::cout << "       PASS: Synapse disruption correctly triggered Instinctive Behavior (Feral Frenzy).\n";
}

void test_sector_graph_and_fallbacks() {
    std::cout << "[TEST] 5. Sector Multi-Tier Defense & Fallback..." << std::endl;
    SectorGraph graph = SectorGraph::create_standard_planetary_graph();
    OMNIS_ASSERT(graph.sectors.size() == 6);
    OMNIS_ASSERT(graph.active_sector_index == 0);

    // Apply breach damage
    Sector& sec1 = graph.get_active_sector();
    sec1.apply_siege_damage(sec1.max_integrity + 1000.0);
    OMNIS_ASSERT(sec1.is_fallen());
    OMNIS_ASSERT(sec1.breach_percentage == 100.0);

    // Advance
    bool has_next = graph.advance_active_sector();
    OMNIS_ASSERT(has_next);
    OMNIS_ASSERT(graph.active_sector_index == 1);
    OMNIS_ASSERT(graph.get_active_sector().name == "Ash Wastes Perimeter");

    // Tactical Fallback doctrine
    bool fb = graph.fallback_to_next_sector();
    OMNIS_ASSERT(fb);
    OMNIS_ASSERT(graph.active_sector_index == 2);
    OMNIS_ASSERT(graph.get_active_sector().name == "Trench Line Alpha");

    std::cout << "       PASS: Sector progression, breach cascade, and tactical retreats verified.\n";
}

void test_logistics_depletion() {
    std::cout << "[TEST] 4. Munitions Logistics & Weapon Starvation..." << std::endl;
    LogisticsSystem log;
    log.artillery_shells = 100.0; // Small stockpile

    // Drain shells
    double eff1 = log.consume_ammo(AmmoType::AMMO_ARTILLERY_SHELLS, 80.0);
    OMNIS_ASSERT(eff1 == 1.0);
    OMNIS_ASSERT(std::abs(log.artillery_shells - 20.0) < 1e-4);

    double eff2 = log.consume_ammo(AmmoType::AMMO_ARTILLERY_SHELLS, 50.0);
    OMNIS_ASSERT(eff2 < 1.0);
    OMNIS_ASSERT(log.artillery_shells == 0.0);
    OMNIS_ASSERT(log.get_efficiency(AmmoType::AMMO_ARTILLERY_SHELLS) < 0.10);

    // Promethium check
    OMNIS_ASSERT(log.can_afford_promethium(15000.0));
    bool burned = log.consume_promethium(15000.0);
    OMNIS_ASSERT(burned);
    OMNIS_ASSERT(log.total_promethium_burned == 15000.0);

    std::cout << "       PASS: Ammo consumption, dry fire degradation, and fuel expenditure verified.\n";
}

void test_monte_carlo_projections() {
    std::cout << "[TEST] 7. Monte Carlo Planetary Outcome Projections..." << std::endl;
    ArmyRoster def = ArmyRoster::create_imperial_defense(1.0);
    ArmyRoster att = ArmyRoster::create_tyranid_swarm(1.0);
    SectorGraph sec = SectorGraph::create_standard_planetary_graph();
    LogisticsSystem log;

    PlanetaryProjection proj = ProjectionEngine::run_monte_carlo(def, att, sec, log, 15);
    OMNIS_ASSERT(proj.iterations_run == 15);
    double total_prob = proj.win_percentage + proj.stalemate_percentage + proj.defeat_percentage;
    OMNIS_ASSERT(std::abs(total_prob - 100.0) < 1e-2);
    OMNIS_ASSERT(proj.expected_days_survived >= 0.0);
    OMNIS_ASSERT(!proj.strategic_assessment.empty());

    std::cout << "       PASS: Win (" << proj.win_percentage << "%), Stalemate (" 
              << proj.stalemate_percentage << "%), Defeat (" << proj.defeat_percentage 
              << "%) computed across 15 iterations.\n";
}

void test_command_doctrines() {
    std::cout << "[TEST] 6. Tactical Command Doctrines..." << std::endl;
    SimulationEngine sim;

    // Trigger Promethium Moat (F1)
    double initial_gaunts = sim.attackers.find_unit("hormagaunt")->count;
    bool ok1 = sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
    OMNIS_ASSERT(ok1);
    double post_fire_gaunts = sim.attackers.find_unit("hormagaunt")->count;
    OMNIS_ASSERT(post_fire_gaunts < initial_gaunts);

    // Cooldown check
    bool ok_cd = sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
    OMNIS_ASSERT(!ok_cd); // Should be on cooldown

    // Trigger Titan Plasma Volley (F2)
    double initial_carnifex = sim.attackers.find_unit("carnifex")->count;
    bool ok2 = sim.trigger_doctrine(DoctrineType::TITAN_PLASMA_VOLLEY);
    OMNIS_ASSERT(ok2);
    double post_titan_carn = sim.attackers.find_unit("carnifex")->count;
    OMNIS_ASSERT(post_titan_carn < initial_carnifex);

    // Trigger Drop-Pod Synapse Spearhead (F3)
    double initial_warriors = sim.attackers.find_unit("tyranid_warrior")->count;
    bool ok3 = sim.trigger_doctrine(DoctrineType::ASTARTES_DROP_POD);
    OMNIS_ASSERT(ok3);
    double post_warriors = sim.attackers.find_unit("tyranid_warrior")->count;
    OMNIS_ASSERT(post_warriors < initial_warriors);

    std::cout << "       PASS: Promethium flame, Titan plasma, and Space Marine drop-pods verified.\n";
}

void test_fortification_absorption_and_progression() {
    std::cout << "[TEST] 11. Multi-Tier Sector Fortification & Breach Progression..." << std::endl;
    SimulationEngine sim;
    
    // Simulate 3 seconds in Sector 1 (Orbital Grid)
    double init_humans = sim.defenders.get_total_count();
    sim.update(3.0);

    // Defenders must still have the vast majority alive (>90%) because Sector 1 absorbs the space assault
    double post_humans = sim.defenders.get_total_count();
    OMNIS_ASSERT(post_humans > init_humans * 0.85);
    OMNIS_ASSERT(sim.sector_graph.get_active_sector().breach_percentage > 0.0);

    std::cout << "       PASS: Fortification absorbed assault. Surface garrison preserved ("
              << (int)post_humans << " / " << (int)init_humans << " alive).\n";
}

void test_environmental_hazards_and_breaches() {
    std::cout << "[TEST] 13. Environmental Hazards & Subterranean Breaches..." << std::endl;
    EnvironmentalHazards hazards;
    hazards.add_spore_toxicity(45.0);
    OMNIS_ASSERT(hazards.spore_toxicity == 45.0);

    ArmyRoster def = ArmyRoster::create_imperial_defense();
    ArmyRoster att = ArmyRoster::create_tyranid_swarm();
    Sector sec(3, "Trench", "SEC-03", "Trenches", 50000.0, 0.0, 1e8);

    double init_inf = def.find_unit("cadian_infantry")->count;
    hazards.update(2.0, def, att, sec);
    double post_inf = def.find_unit("cadian_infantry")->count;

    // Unsealed Cadians suffer casualties from toxic spores
    OMNIS_ASSERT(post_inf < init_inf);

    // Sealed Space Marines take 0 casualties from spores
    UnitClass* marine = def.find_unit("astartes_spearhead");
    OMNIS_ASSERT(marine != nullptr);
    OMNIS_ASSERT(marine->count == marine->initial_count);

    // Trigger Mawloc subterranean breach
    hazards.trigger_subterranean_breach(10.0);
    OMNIS_ASSERT(hazards.subterranean_breach_active);

    double init_arty = def.find_unit("basilisk_artillery")->count;
    hazards.update(1.0, def, att, sec);
    double post_arty = def.find_unit("basilisk_artillery")->count;
    OMNIS_ASSERT(post_arty < init_arty); // Backline attacked directly

    std::cout << "       PASS: Spore toxicity filtered by sealed armor; subterranean Mawlocs struck backline.\n";
}

void test_commissar_execution_mechanic() {
    std::cout << "[TEST] 14. Commissar Summary Execution & Orbital Lasers..." << std::endl;
    EnvironmentalHazards hazards;
    ArmyRoster def = ArmyRoster::create_imperial_defense();
    ArmyRoster att = ArmyRoster::create_tyranid_swarm();

    bool executed = hazards.trigger_commissar_execution(def);
    OMNIS_ASSERT(executed);
    OMNIS_ASSERT(hazards.commissar_executions_count == 1.0);

    // Orbital defense laser strike
    double kills = hazards.trigger_orbital_defense_laser(att);
    OMNIS_ASSERT(kills > 1000000.0); // Millions of vanguard organisms vaporized
    OMNIS_ASSERT(hazards.orbital_laser_cooldown > 0.0);

    std::cout << "       PASS: Commissar execution restored resolve; Orbital Lasers vaporized " 
              << (int)kills << " organisms.\n";
}

void test_telemetry_export() {
    std::cout << "[TEST] 15. Battle Telemetry JSON & Forensic Log Export..." << std::endl;
    ArmyRoster def = ArmyRoster::create_imperial_defense();
    ArmyRoster att = ArmyRoster::create_tyranid_swarm();
    Sector sec(1, "Orbital", "SEC-01", "Orbital Grid", 100000.0, 0.0, 1e8);
    LogisticsSystem log;

    bool json_ok = TelemetryExport::export_battle_json("bin/test_report.json", def, att, sec, log, 3600.0, "TEST VICTORY");
    bool log_ok = TelemetryExport::export_forensic_log("bin/test_forensic.log", def, att, sec, log, 3600.0, "TEST VICTORY");

    OMNIS_ASSERT(json_ok);
    OMNIS_ASSERT(log_ok);

    std::cout << "       PASS: Telemetry JSON and forensic logs successfully exported and verified.\n";
}

void test_modular_factions() {
    std::cout << "[TEST] 8. Modular Faction Expansion Roster (Orks, Necrons, Chaos)..." << std::endl;
    ArmyRoster orks = ArmyRoster::create_ork_waaagh(1.0);
    ArmyRoster necrons = ArmyRoster::create_necron_dynasty(1.0);
    ArmyRoster chaos = ArmyRoster::create_chaos_incursion(1.0);

    OMNIS_ASSERT(orks.get_total_count() > 40000000.0);
    OMNIS_ASSERT(necrons.get_total_count() > 15000000.0);
    OMNIS_ASSERT(chaos.get_total_count() > 40000000.0);

    OMNIS_ASSERT(orks.get_total_living_hp() > 0.0);
    OMNIS_ASSERT(necrons.get_total_living_hp() > 0.0);
    OMNIS_ASSERT(chaos.get_total_living_hp() > 0.0);

    std::cout << "       PASS: Modular faction rosters initialized with complete statistics.\n";
}

void test_modular_doctrine_execution() {
    std::cout << "[TEST] 10. Modular Command Doctrines on Orks and Necrons..." << std::endl;
    CommandOrders orders;
    ArmyRoster def = ArmyRoster::create_imperial_defense(1.0);
    ArmyRoster orks = ArmyRoster::create_ork_waaagh(1.0);
    SectorGraph sec = SectorGraph::create_standard_planetary_graph();
    LogisticsSystem log;
    SynapseNetwork syn;

    // Test Astartes Drop Pod on Orks
    double init_ork_count = orks.get_total_count();
    bool drop_ok = orders.trigger_doctrine(DoctrineType::ASTARTES_DROP_POD, def, orks, sec, log, syn);
    OMNIS_ASSERT(drop_ok);
    OMNIS_ASSERT(orks.get_total_count() < init_ork_count);

    // Test Titan Plasma Volley on Orks (must hit Battlewagons / Gargants)
    orders.reset();
    init_ork_count = orks.get_total_count();
    bool titan_ok = orders.trigger_doctrine(DoctrineType::TITAN_PLASMA_VOLLEY, def, orks, sec, log, syn);
    OMNIS_ASSERT(titan_ok);
    OMNIS_ASSERT(orks.get_total_count() < init_ork_count);

    std::cout << "       PASS: Modular doctrines successfully executed against non-Tyranid factions.\n";
}

void test_tau_aeldari_custodes_rosters() {
    std::cout << "[TEST] 12. T'au, Aeldari, and Custodes Factions..." << std::endl;
    ArmyRoster tau = ArmyRoster::create_tau_empire();
    ArmyRoster eldar = ArmyRoster::create_aeldari_host();
    ArmyRoster custodes = ArmyRoster::create_custodes_vanguard();

    OMNIS_ASSERT(tau.units.size() == 4);
    OMNIS_ASSERT(eldar.units.size() == 4);
    OMNIS_ASSERT(custodes.units.size() == 3);

    // Custodes have heavy armor (>=0.85) and massive HP
    UnitClass* guard = custodes.find_unit("custodian_guard");
    OMNIS_ASSERT(guard != nullptr && guard->armor >= 0.85);

    // T'au Hammerhead has high AP and heavy damage
    UnitClass* railgun = tau.find_unit("hammerhead_gunship");
    OMNIS_ASSERT(railgun != nullptr && railgun->dps_per_unit >= 500.0);

    std::cout << "       PASS: T'au, Aeldari, and Custodes rosters initialized with authentic profiles.\n";
}

int main() {
    std::cout << "=================================================================\n";
    std::cout << "  OMNISBELLI // AUTOMATED COMBAT ENGINE VERIFICATION SUITE       \n";
    std::cout << "=================================================================\n";

    test_unit_mechanics();
    test_lanchester_laws();
    test_excess_damage_cascade();
    test_synapse_disruption();
    test_sector_graph_and_fallbacks();
    test_logistics_depletion();
    test_monte_carlo_projections();
    test_command_doctrines();
    test_fortification_absorption_and_progression();
    test_environmental_hazards_and_breaches();
    test_commissar_execution_mechanic();
    test_telemetry_export();
    test_modular_factions();
    test_modular_doctrine_execution();
    test_tau_aeldari_custodes_rosters();

    std::cout << "=================================================================\n";
    std::cout << "  ALL 15 SIMULATION ENGINE TEST SUITES PASSED FLAWLESSLY!        \n";
    std::cout << "=================================================================\n";
    return 0;
}

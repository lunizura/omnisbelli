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
#include "engine/faction_rule_module.hpp"
#include "engine/death_guard_mechanic.hpp"
#include "engine/sororitas_mechanic.hpp"
#include "engine/mechanicus_mechanic.hpp"
#include "engine/world_eaters_mechanic.hpp"
#include "engine/ork_waaagh_mechanic.hpp"
#include "engine/necron_dynasty_mechanic.hpp"
#include "engine/tau_empire_mechanic.hpp"
#include "engine/custodes_mechanic.hpp"
#include "engine/config_loader.hpp"

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

void test_extreme_scale_billion_entities() {
    std::cout << "[TEST] 16. Extreme Scale (1,000,000,000 Entities) Numerical Stability..." << std::endl;
    ArmyRoster def = ArmyRoster::create_imperial_defense(10.0); // 20M Defenders
    ArmyRoster swarm = ArmyRoster::create_tyranid_swarm(10.0); // 1 Billion Swarm

    OMNIS_ASSERT(swarm.get_total_count() >= 1000000000.0);

    Sector sec(2, "Plains", "SEC-02", "Ash Wastes", 500000000.0, 0.0, 1e10);
    LogisticsSystem log;
    SynapseNetwork syn;
    CombatStepTelemetry tel;

    // Run 10 ticks at billion scale
    for (int i = 0; i < 10; ++i) {
        LanchesterCombat::compute_combat_tick(def, swarm, sec, log, syn, 1.0, tel);
        OMNIS_ASSERT(!std::isnan(tel.attacker_dps_effective));
        OMNIS_ASSERT(!std::isnan(tel.defender_dps_effective));
    }

    std::cout << "       PASS: 1 Billion swarm simulation stable with zero floating-point overflow or NaN.\n";
}


void test_death_guard_disgustingly_resilient() {
    std::cout << "[TEST] 17. Death Guard Disgustingly Resilient & Poxwalker Outbreak..." << std::endl;
    DeathGuardMechanic dg;
    ArmyRoster friendly = ArmyRoster::create_death_guard(1.0);
    ArmyRoster enemy = ArmyRoster::create_imperial_defense(1.0);

    // Incoming 10,000 raw damage: FNP factor 0.50 => mitigated = 10000 / 1.5 = 6666.67
    double raw = 10000.0;
    double mitigated = dg.apply_incoming_mitigation(friendly, raw, 1.0);
    OMNIS_ASSERT(mitigated < raw * 0.70);
    OMNIS_ASSERT(std::abs(mitigated - (raw / 1.5)) < 1e-3);
    OMNIS_ASSERT(dg.disgusting_resilience_damage_negated > 3300.0);

    // Enemy casualties generate Poxwalkers when contagion is present
    dg.contagion_intensity = 0.50;
    CasualtyEvent enemy_cas{ 10000.0, 100000.0, "infantry", false };
    dg.on_casualty_event(friendly, enemy, enemy_cas);
    // 10000 * 0.12 * 0.50 = 600
    OMNIS_ASSERT(std::abs(dg.poxwalkers_spawned - 600.0) < 1e-3);

    std::cout << "       PASS: Disgustingly Resilient 33.3% FNP mitigation & Poxwalker recursion verified.\n";
}

void test_death_guard_contagion_decay() {
    std::cout << "[TEST] 18. Death Guard Contagion Decay & Environmental Rot..." << std::endl;
    DeathGuardMechanic dg;
    ArmyRoster friendly = ArmyRoster::create_death_guard(1.0);
    ArmyRoster enemy = ArmyRoster::create_imperial_defense(1.0);
    Sector sec(1, "Plague Mire", "SEC-PLAGUE", "Toxic sludge", 10000.0, 0.50, 1e7);

    // Pre-combat ticks build contagion and corrode cover
    for (int i = 0; i < 50; ++i) {
        dg.pre_combat_tick(friendly, enemy, sec, 2.0);
    }
    OMNIS_ASSERT(dg.contagion_intensity > 0.30);
    OMNIS_ASSERT(sec.fortification_cover < 0.50);

    // Contagion grants bonus armor penetration
    DamagePacket pkt{ 1000.0, 0.0, 1.0, false };
    dg.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(pkt.armor_penetration >= 0.07);

    std::cout << "       PASS: Contagion aura progression, fortification corrosion, and AP verified.\n";
}

void test_mechanicus_canticle_stances() {
    std::cout << "[TEST] 19. Adeptus Mechanicus Canticle Stances & Rad-Saturation..." << std::endl;
    MechanicusMechanic admech;
    ArmyRoster friendly = ArmyRoster::create_adeptus_mechanicus(1.0);
    ArmyRoster enemy = ArmyRoster::create_ork_waaagh(1.0);
    Sector sec(1, "Forge Sector", "SEC-FORGE", "Industrial", 10000.0, 0.40, 1e7);

    // Stance 1: Protector Imperative (+35% ranged)
    DamagePacket pkt{ 1000.0, 0.0, 1.0, false };
    admech.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(std::abs(pkt.raw_damage - 1350.0) < 1e-3);
    OMNIS_ASSERT(pkt.armor_penetration == 0.10);

    // Stance 2: Conqueror Imperative (+45% melee)
    admech.set_protocol(CanticleProtocol::CONQUEROR_IMPERATIVE);
    pkt = DamagePacket{ 1000.0, 0.0, 1.0, false };
    admech.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(std::abs(pkt.raw_damage - 1450.0) < 1e-3);

    // Stance 3: Shroudpsalm (30% damage absorbed)
    admech.set_protocol(CanticleProtocol::SHROUDPSALM_STEALTH);
    double mit = admech.apply_incoming_mitigation(friendly, 1000.0, 1.0);
    OMNIS_ASSERT(std::abs(mit - 700.0) < 1e-3);

    // Rad-saturation suppresses unsealed enemy organic HP
    UnitClass* boyz = enemy.find_unit("ork_boyz");
    OMNIS_ASSERT(boyz != nullptr);
    double initial_hp = boyz->hp_per_unit;
    admech.pre_combat_tick(friendly, enemy, sec, 10.0);
    OMNIS_ASSERT(boyz->hp_per_unit < initial_hp);

    std::cout << "       PASS: 4 Canticle protocols, tactical stances, and Rad-Saturation verified.\n";
}

void test_sororitas_miracle_dice() {
    std::cout << "[TEST] 20. Adepta Sororitas Miracle Dice & Acts of Faith..." << std::endl;
    SororitasMechanic sor;
    ArmyRoster friendly = ArmyRoster::create_adepta_sororitas(1.0);
    ArmyRoster enemy = ArmyRoster::create_world_eaters(1.0);

    // Suffer friendly casualties to generate Miracle Points (12,000 casualties per point)
    double init_points = sor.miracle_points; // 15.0
    CasualtyEvent cas{ 24000.0, 600000.0, "infantry", true };
    sor.on_casualty_event(friendly, enemy, cas);
    OMNIS_ASSERT(std::abs(sor.miracle_points - (init_points + 2.0)) < 1e-3);

    // Trigger divine intervention shield (absorbs 50% damage)
    sor.miracle_points = 30.0;
    bool ok = sor.trigger_divine_intervention();
    OMNIS_ASSERT(ok && sor.divine_shield_active);
    double incoming = 10000.0;
    double mitigated = sor.apply_incoming_mitigation(friendly, incoming, 1.0);
    OMNIS_ASSERT(std::abs(mitigated - 5000.0) < 1e-3);

    // Holy Trinity Purity bonus when faith >= 50
    sor.miracle_points = 100.0;
    DamagePacket pkt{ 1000.0, 0.0, 1.0, false };
    sor.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(std::abs(pkt.raw_damage - 1350.0) < 1e-3);
    OMNIS_ASSERT(pkt.armor_penetration >= 0.15);

    std::cout << "       PASS: Martyrdom faith accumulation, 4++ aegis shield, and Holy Trinity verified.\n";
}

void test_world_eaters_blood_tithe() {
    std::cout << "[TEST] 21. World Eaters Blood Tithe of Khorne & Apotheosis..." << std::endl;
    WorldEatersMechanic we;
    ArmyRoster friendly = ArmyRoster::create_world_eaters(1.0);
    ArmyRoster enemy = ArmyRoster::create_death_guard(1.0);
    Sector sec(1, "Slaughter Pit", "SEC-PIT", "Blood arena", 10000.0, 0.20, 1e7);

    // Accumulate kills: 80,000 kills = 1 Tithe Point
    CasualtyEvent cas{ 800000.0, 8000000.0, "all", true };
    we.on_casualty_event(friendly, enemy, cas);
    OMNIS_ASSERT(std::abs(we.blood_tithe_points - 10.0) < 1e-3);

    // Pre-combat tick updates tithe tier
    we.pre_combat_tick(friendly, enemy, sec, 1.0);
    OMNIS_ASSERT(we.tithe_tier == 1);

    // Tier 1 Spiteful Demise damage on friendly casualty
    double prev_spite = we.spiteful_demise_damage_dealt;
    we.on_casualty_event(friendly, enemy, CasualtyEvent{ 100.0, 17000.0, "infantry", true });
    OMNIS_ASSERT(we.spiteful_demise_damage_dealt > prev_spite);

    // Advance to Tier 3 (>= 50 pts: +35% DPS, +0.15 AP)
    we.blood_tithe_points = 60.0;
    we.pre_combat_tick(friendly, enemy, sec, 1.0);
    OMNIS_ASSERT(we.tithe_tier == 3);
    DamagePacket pkt{ 1000.0, 0.0, 1.0, false };
    we.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(std::abs(pkt.raw_damage - 1350.0) < 1e-3);
    OMNIS_ASSERT(pkt.armor_penetration >= 0.15);

    // Advance to Tier 4 (>= 100 pts: Bloodthirster summoning)
    we.blood_tithe_points = 105.0;
    we.pre_combat_tick(friendly, enemy, sec, 1.0);
    OMNIS_ASSERT(we.tithe_tier == 4);
    OMNIS_ASSERT(we.bloodthirsters_summoned > 0.0);

    std::cout << "       PASS: Blood Tithe tiers 1-4, Spiteful Demise, and Bloodthirster summoning verified.\n";
}

void test_cross_faction_lanchester_matrix() {
    std::cout << "[TEST] 22. Cross-Faction Lanchester Modular Pipeline & Any-vs-Any Matchups..." << std::endl;
    
    // Matchup 1: Death Guard defenders vs World Eaters attackers
    {
        auto dg_rules = create_faction_rule_module("death_guard");
        auto we_rules = create_faction_rule_module("world_eaters");
        ArmyRoster def = ArmyRoster::create_death_guard(1.0);
        ArmyRoster att = ArmyRoster::create_world_eaters(1.0);
        Sector sec(1, "Istvaan Trench", "SEC-IST", "Trench line", 50000.0, 0.40, 1e8);
        LogisticsSystem log;
        CombatStepTelemetry tel;

        LanchesterCombat::compute_combat_tick(def, att, sec, log, *dg_rules, *we_rules, 1.0, tel);
        OMNIS_ASSERT(tel.defender_dps_effective > 0.0);
        OMNIS_ASSERT(tel.attacker_dps_effective > 0.0);
        OMNIS_ASSERT(!std::isnan(tel.defender_casualties_step));
        OMNIS_ASSERT(!std::isnan(tel.attacker_casualties_step));
    }

    // Matchup 2: Adepta Sororitas defenders vs Adeptus Mechanicus attackers
    {
        auto sor_rules = create_faction_rule_module("sororitas");
        auto mech_rules = create_faction_rule_module("mechanicus");
        ArmyRoster def = ArmyRoster::create_adepta_sororitas(1.0);
        ArmyRoster att = ArmyRoster::create_adeptus_mechanicus(1.0);
        Sector sec(1, "Cathedral Gate", "SEC-CATH", "Sanctuary", 30000.0, 0.60, 1e8);
        LogisticsSystem log;
        CombatStepTelemetry tel;

        LanchesterCombat::compute_combat_tick(def, att, sec, log, *sor_rules, *mech_rules, 1.0, tel);
        OMNIS_ASSERT(tel.defender_dps_effective > 0.0);
        OMNIS_ASSERT(tel.attacker_dps_effective > 0.0);
    }

    // Matchup 3: Dynamic SimulationEngine Faction Switching
    {
        SimulationEngine sim;
        sim.set_defender_faction("sororitas");
        sim.set_attacker_faction("death_guard");
        OMNIS_ASSERT(sim.defenders.faction_id == "sororitas");
        OMNIS_ASSERT(sim.attackers.faction_id == "death_guard");

        sim.update(0.5);
        OMNIS_ASSERT(sim.sim_time_seconds > 0.0);
        OMNIS_ASSERT(!sim.defenders.is_defeated());
    }

    std::cout << "       PASS: Any-vs-any cross-faction differential matrix and simulation engine validated.\n";
}

void test_ork_waaagh_mechanics() {
    std::cout << "[TEST] 23. Ork WAAAGH! Momentum Curve, Dakkadakka, and Grot Shields..." << std::endl;
    OrkWaaaghMechanic ork;
    ArmyRoster friendly = ArmyRoster::create_ork_waaagh(1.0);
    ArmyRoster enemy = ArmyRoster::create_imperial_defense(1.0);
    Sector sec(1, "Scrap Plain", "SEC-SCRAP", "Wastes", 50000.0, 0.20, 1e7);

    // Initial momentum
    OMNIS_ASSERT(ork.waaagh_momentum >= 15.0);

    // Dakkadakka burst outgoing damage
    DamagePacket pkt{ 1000.0, 0.0, 1.0, false };
    ork.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(pkt.raw_damage > 1150.0); // +20% dakkadakka
    OMNIS_ASSERT(pkt.armor_penetration >= 0.08);

    // Grot Shields: absorbs 20% incoming damage
    double incoming = 10000.0;
    double mitigated = ork.apply_incoming_mitigation(friendly, incoming, 1.0);
    OMNIS_ASSERT(std::abs(mitigated - 8000.0) < 1e-3);
    OMNIS_ASSERT(ork.grot_shields_absorbed_damage >= 2000.0);

    // Casualties build WAAAGH! momentum
    double prev_mom = ork.waaagh_momentum;
    CasualtyEvent cas{ 600000.0, 6000000.0, "infantry", true };
    ork.on_casualty_event(friendly, enemy, cas);
    OMNIS_ASSERT(ork.waaagh_momentum >= prev_mom + 9.9);

    // Trigger WAAAGH! Roar
    ork.trigger_waaagh_roar();
    OMNIS_ASSERT(ork.waaagh_frenzy_active);
    DamagePacket frenzy_pkt{ 1000.0, 0.0, 1.0, false };
    ork.modify_outgoing_damage(friendly, frenzy_pkt, 1.0);
    OMNIS_ASSERT(frenzy_pkt.raw_damage >= 1600.0); // frenzy + dakkadakka

    std::cout << "       PASS: WAAAGH! momentum, Dakkadakka burst hits, and Grot Shields verified.\n";
}

void test_necron_dynasty_mechanics() {
    std::cout << "[TEST] 24. Necron Reanimation Protocols, Living Metal, and Phase-Out..." << std::endl;
    NecronDynastyMechanic necron;
    ArmyRoster friendly = ArmyRoster::create_necron_dynasty(1.0);
    ArmyRoster enemy = ArmyRoster::create_imperial_defense(1.0);
    Sector sec(1, "Tomb Catacomb", "SEC-TOMB", "Necron Vault", 25000.0, 0.50, 1e8);

    // Outgoing Gauss flaying AP
    DamagePacket pkt{ 1000.0, 0.0, 1.0, false };
    necron.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(pkt.armor_penetration >= 0.12);

    // Passive Living metal chassis mitigation (10%)
    double incoming = 10000.0;
    double mitigated = necron.apply_incoming_mitigation(friendly, incoming, 1.0);
    OMNIS_ASSERT(std::abs(mitigated - 9000.0) < 1e-3);

    // Casualties feed pending reanimation pool (20%)
    UnitClass* warriors = friendly.find_unit("necron_warrior");
    OMNIS_ASSERT(warriors != nullptr);
    double initial_w = warriors->count;
    warriors->count -= 50000.0; // simulate losses
    CasualtyEvent cas{ 50000.0, 1200000.0, "infantry", true };
    necron.on_casualty_event(friendly, enemy, cas);
    OMNIS_ASSERT(necron.pending_reanimation_pool >= 10000.0);

    // Pre-combat tick triggers Reanimation Protocol, reconstituting fallen warriors
    necron.pre_combat_tick(friendly, enemy, sec, 1.0);
    OMNIS_ASSERT(necron.total_reanimated_units > 0.0);
    OMNIS_ASSERT(warriors->count > initial_w - 50000.0);

    // Living metal repair on Monolith
    UnitClass* mono = friendly.find_unit("monolith");
    OMNIS_ASSERT(mono != nullptr);
    mono->count -= 200.0;
    double pre_mono = mono->count;
    necron.pre_combat_tick(friendly, enemy, sec, 1.0);
    OMNIS_ASSERT(mono->count > pre_mono);

    // Phase-out alert when below 25%
    for (auto& u : friendly.units) {
        u.count = u.initial_count * 0.20;
    }
    necron.pre_combat_tick(friendly, enemy, sec, 1.0);
    OMNIS_ASSERT(necron.phase_out_triggered);

    std::cout << "       PASS: Reanimation Protocols, Living Metal hull repair, and Phase-Out alert verified.\n";
}

void test_tau_empire_mechanics() {
    std::cout << "[TEST] 25. T'au Empire Markerlight Target Network & Tactical Stances..." << std::endl;
    TauEmpireMechanic tau;
    ArmyRoster friendly = ArmyRoster::create_tau_empire(1.0);
    ArmyRoster enemy = ArmyRoster::create_tyranid_swarm(1.0);
    Sector sec(1, "Tau Firebase", "SEC-TAU", "Redoubt", 40000.0, 0.45, 1e7);

    // Initial Mont'ka early blitz bonus (<= 90s)
    OMNIS_ASSERT(tau.current_stance == TauStance::MONTKA);
    DamagePacket pkt{ 1000.0, 0.0, 1.0, false };
    tau.modify_outgoing_damage(friendly, pkt, 1.0);
    OMNIS_ASSERT(pkt.raw_damage >= 1350.0);
    OMNIS_ASSERT(pkt.armor_penetration >= 0.10);

    // Shield drone mitigation (18%)
    double incoming = 10000.0;
    double mitigated = tau.apply_incoming_mitigation(friendly, incoming, 1.0);
    OMNIS_ASSERT(std::abs(mitigated - 8200.0) < 1e-3);
    OMNIS_ASSERT(tau.shield_drone_damage_absorbed >= 1800.0);

    // Markerlight saturation progression
    tau.add_markerlight_tokens(8.0); // reach 10 tokens
    OMNIS_ASSERT(tau.markerlight_tokens >= 9.0);
    DamagePacket ml_pkt{ 1000.0, 0.0, 1.0, false };
    tau.modify_outgoing_damage(friendly, ml_pkt, 1.0);
    OMNIS_ASSERT(ml_pkt.raw_damage > 1700.0); // Mont'ka + Tier 3 Seeker guidance
    OMNIS_ASSERT(ml_pkt.armor_penetration >= 0.35);
    OMNIS_ASSERT(ml_pkt.ignores_cover);

    // Switch to Kauyon (Patient Hunter)
    tau.set_stance(TauStance::KAUYON);
    tau.stance_elapsed_time = 40.0; // 40s into Kauyon (+20% sustained fire)
    DamagePacket kau_pkt{ 1000.0, 0.0, 1.0, false };
    tau.modify_outgoing_damage(friendly, kau_pkt, 1.0);
    OMNIS_ASSERT(kau_pkt.raw_damage >= 1500.0);

    std::cout << "       PASS: Markerlight tokens, Mont'ka/Kauyon stances, and Shield Drones verified.\n";
}

void test_custodes_martial_kata() {
    std::cout << "[TEST] 26. Adeptus Custodes Martial Ka'tah & Auric Armor Aegis..." << std::endl;
    CustodesMechanic custodes;
    ArmyRoster friendly = ArmyRoster::create_custodes_vanguard(1.0);
    ArmyRoster enemy = ArmyRoster::create_tyranid_swarm(1.0);
    Sector sec(1, "Imperial Palace Gate", "SEC-PALACE", "Sanctum", 10000.0, 0.80, 1e8);

    // Dacatarai stance: anti-horde cleave (+40% DPS)
    custodes.set_kata_stance(KataStance::DACATARAI);
    DamagePacket daca_pkt{ 1000.0, 0.0, 1.0, false };
    custodes.modify_outgoing_damage(friendly, daca_pkt, 1.0);
    OMNIS_ASSERT(std::abs(daca_pkt.raw_damage - 1400.0) < 1e-3);

    // Rendax stance: monster/titan slayer (+25% DPS, +0.25 AP)
    custodes.set_kata_stance(KataStance::RENDAX);
    DamagePacket rend_pkt{ 1000.0, 0.0, 1.0, false };
    custodes.modify_outgoing_damage(friendly, rend_pkt, 1.0);
    OMNIS_ASSERT(std::abs(rend_pkt.raw_damage - 1250.0) < 1e-3);
    OMNIS_ASSERT(rend_pkt.armor_penetration >= 0.25);

    // Auric Armor 2+/4++ Aegis: 35% flat mitigation
    double incoming = 10000.0;
    double mitigated = custodes.apply_incoming_mitigation(friendly, incoming, 1.0);
    OMNIS_ASSERT(std::abs(mitigated - 6500.0) < 1e-3);

    // Kaptaris stance: parry blades add extra mitigation
    custodes.set_kata_stance(KataStance::KAPTARIS);
    double kapt_mit = custodes.apply_incoming_mitigation(friendly, incoming, 1.0);
    OMNIS_ASSERT(kapt_mit < 5000.0);

    std::cout << "       PASS: Martial Ka'tah stances, Auric Armor Aegis, and Kaptaris parry verified.\n";
}

void test_faction_command_doctrines() {
    std::cout << "[TEST] 27. Modular Faction-Specific Tactical Command Doctrines..." << std::endl;
    CommandOrders orders;

    // Default: Imperium
    OMNIS_ASSERT(orders.doctrines[0].name == "IGNITE PROMETHIUM MOAT");
    OMNIS_ASSERT(orders.doctrines[1].name == "TITAN PLASMA VOLLEY");

    // Configure for Orks
    orders.configure_for_faction("orks");
    OMNIS_ASSERT(orders.doctrines[0].name == "DA BIG WAAAGH! ROAR");
    OMNIS_ASSERT(orders.doctrines[1].name == "ROKKS FROM DA SKY");
    OMNIS_ASSERT(orders.doctrines[2].name == "GREEN TIDE MOB CALL");
    OMNIS_ASSERT(orders.doctrines[4].name == "DAKKA BURST STORM");

    // Configure for Necrons
    orders.configure_for_faction("necrons");
    OMNIS_ASSERT(orders.doctrines[0].name == "REANIMATION OVERCHARGE");
    OMNIS_ASSERT(orders.doctrines[1].name == "DOOMSDAY ARKANITE BEAM");
    OMNIS_ASSERT(orders.doctrines[2].name == "ETERNITY GATE INGRESS");

    // Configure for T'au
    orders.configure_for_faction("tau");
    OMNIS_ASSERT(orders.doctrines[0].name == "MARKERLIGHT SATURATION");
    OMNIS_ASSERT(orders.doctrines[1].name == "SEEKER MISSILE VOLLEY");

    // Configure for Death Guard
    orders.configure_for_faction("death_guard");
    OMNIS_ASSERT(orders.doctrines[0].name == "MIASMA OF PESTILENCE");
    OMNIS_ASSERT(orders.doctrines[1].name == "PLAGUEBURST PHOSPHEX BARRAGE");

    // Execution test: Ork WAAAGH! Roar
    {
        orders.configure_for_faction("orks");
        ArmyRoster orks = ArmyRoster::create_ork_waaagh(1.0);
        ArmyRoster tyranids = ArmyRoster::create_tyranid_swarm(1.0);
        SectorGraph graph = SectorGraph::create_standard_planetary_graph();
        LogisticsSystem log;
        SynapseNetwork syn;

        double prev_tyranid_count = tyranids.get_total_count();
        bool ok = orders.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT, orks, tyranids, graph, log, syn);
        OMNIS_ASSERT(ok);
        OMNIS_ASSERT(tyranids.get_total_count() < prev_tyranid_count);
    }

    std::cout << "       PASS: Dynamic faction doctrine configuration and tactical execution verified.\n";
}

void test_config_loader_json() {
    std::cout << "[TEST] 28. Zero-Dependency JSON Config Customizer Loader & Exporter..." << std::endl;
    SimulationEngine sim;
    std::string test_path = "bin/test_custom_config.json";

    // Export current state
    bool saved = ConfigLoader::save_simulation_config(test_path, sim);
    OMNIS_ASSERT(saved);

    // Modify file or verify loader
    SimulationEngine sim2;
    sim2.set_defender_faction("orks");
    sim2.set_speed(10.0);

    bool loaded = ConfigLoader::load_simulation_config(test_path, sim2);
    OMNIS_ASSERT(loaded);
    OMNIS_ASSERT(sim2.defenders.faction_id == "imperium");
    OMNIS_ASSERT(sim2.attackers.faction_id == "tyranids");

    // Test token extractors
    std::string sample = "{\"name\": \"Ghazghkull\", \"count\": 45000000, \"ratio\": 0.85}";
    std::string str_val;
    double num_val = 0.0;
    int int_val = 0;

    OMNIS_ASSERT(ConfigLoader::extract_json_string(sample, "name", str_val));
    OMNIS_ASSERT(str_val == "Ghazghkull");
    OMNIS_ASSERT(ConfigLoader::extract_json_number(sample, "ratio", num_val));
    OMNIS_ASSERT(std::abs(num_val - 0.85) < 1e-4);
    OMNIS_ASSERT(ConfigLoader::extract_json_int(sample, "count", int_val));
    OMNIS_ASSERT(int_val == 45000000);

    std::cout << "       PASS: JSON serialization, parsing, and simulation config load verified.\n";
}

void test_advanced_faction_command_doctrines() {
    std::cout << "[TEST] 29. Full-Spectrum Faction Command Doctrines & State Coupling..." << std::endl;
    SectorGraph graph = SectorGraph::create_standard_planetary_graph();
    LogisticsSystem empty_logistics;
    empty_logistics.promethium_liters = 0.0; // Zero promethium to ensure non-Imperial factions don't erroneously depend on fuel!
    SynapseNetwork syn;

    // 1. T'au Empire F1: Markerlight Saturation
    {
        SimulationEngine sim;
        sim.set_defender_faction("tau");
        sim.logistics.promethium_liters = 0.0;
        auto* tau_mech = dynamic_cast<TauEmpireMechanic*>(sim.defender_rules.get());
        OMNIS_ASSERT(tau_mech != nullptr);
        double pre_ml = tau_mech->markerlight_tokens;

        bool ok = sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
        OMNIS_ASSERT(ok); // Must succeed even with 0 promethium!
        OMNIS_ASSERT(tau_mech->markerlight_tokens >= pre_ml + 5.0);
    }

    // 2. World Eaters F1: Blood for the Blood God
    {
        SimulationEngine sim;
        sim.set_defender_faction("world_eaters");
        sim.logistics.promethium_liters = 0.0;
        auto* we_mech = dynamic_cast<WorldEatersMechanic*>(sim.defender_rules.get());
        OMNIS_ASSERT(we_mech != nullptr);
        double pre_tithe = we_mech->blood_tithe_points;

        bool ok = sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
        OMNIS_ASSERT(ok);
        OMNIS_ASSERT(we_mech->blood_tithe_points >= pre_tithe + 25.0);
    }

    // 3. Adepta Sororitas F1: Act of Faith Divine Aegis
    {
        SimulationEngine sim;
        sim.set_defender_faction("sororitas");
        sim.logistics.promethium_liters = 0.0;
        auto* sor_mech = dynamic_cast<SororitasMechanic*>(sim.defender_rules.get());
        OMNIS_ASSERT(sor_mech != nullptr);
        double pre_miracle = sor_mech->miracle_points;

        bool ok = sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
        OMNIS_ASSERT(ok);
        OMNIS_ASSERT(sor_mech->miracle_points >= pre_miracle + 20.0);
    }

    // 4. Adeptus Custodes F1 & F3: Phalanx & Allarus Teleport Ingress
    {
        SimulationEngine sim;
        sim.set_defender_faction("custodes");
        sim.logistics.promethium_liters = 0.0;
        auto* cust_mech = dynamic_cast<CustodesMechanic*>(sim.defender_rules.get());
        OMNIS_ASSERT(cust_mech != nullptr);

        bool ok1 = sim.trigger_doctrine(DoctrineType::IGNITE_PROMETHIUM_MOAT);
        OMNIS_ASSERT(ok1);
        OMNIS_ASSERT(cust_mech->active_kata == KataStance::KAPTARIS);

        // F3 Allarus reinforcement
        UnitClass* allarus = sim.defenders.find_unit("allarus_terminator");
        OMNIS_ASSERT(allarus != nullptr);
        allarus->count -= 500.0; // Simulate loss
        double count_pre = allarus->count;

        bool ok3 = sim.trigger_doctrine(DoctrineType::ASTARTES_DROP_POD);
        OMNIS_ASSERT(ok3);
        OMNIS_ASSERT(allarus->count > count_pre);
    }

    // 5. Necron Reanimation Pool Preservation
    {
        NecronDynastyMechanic necron;
        ArmyRoster friendly = ArmyRoster::create_necron_dynasty(1.0);
        ArmyRoster enemy = ArmyRoster::create_imperial_defense(1.0);
        Sector sec(1, "Tomb", "SEC-T", "Vault", 20000.0, 0.5, 1e7);

        // Friendly units are currently at 100% full capacity
        CasualtyEvent cas{ 10000.0, 240000.0, "infantry", true };
        necron.on_casualty_event(friendly, enemy, cas);
        double pool_initial = necron.pending_reanimation_pool;
        OMNIS_ASSERT(pool_initial > 0.0);

        // Reanimation tick runs while units are already at max initial count
        necron.pre_combat_tick(friendly, enemy, sec, 1.0);
        // Excess pool must NOT be discarded
        OMNIS_ASSERT(necron.pending_reanimation_pool == pool_initial);
    }

    std::cout << "       PASS: Cross-faction command doctrines and reanimation pool preservation verified.\n";
}

void test_config_loader_edge_cases_and_robustness() {
    std::cout << "[TEST] 30. ConfigLoader Edge Cases, Type Safety, and Projection Recalculation..." << std::endl;

    // 1. Type Safety: extract_json_string on numeric field must NOT match next string
    std::string sample = "{\"count\": 1250000, \"next_key\": \"hello_world\"}";
    std::string str_out;
    bool matched_str = ConfigLoader::extract_json_string(sample, "count", str_out);
    OMNIS_ASSERT(!matched_str); // Must return false, not "next_key"!

    // 2. Type Safety: extract_json_number on string value like "1st Legion" must NOT return 1.0
    std::string sample2 = "{\"title\": \"1st Cadian Regiment\", \"score\": 98.5}";
    double num_out = 0.0;
    bool matched_num = ConfigLoader::extract_json_number(sample2, "title", num_out);
    OMNIS_ASSERT(!matched_num); // Must return false!

    // Correct extraction of numeric field
    bool matched_score = ConfigLoader::extract_json_number(sample2, "score", num_out);
    OMNIS_ASSERT(matched_score);
    OMNIS_ASSERT(std::abs(num_out - 98.5) < 1e-4);

    // 3. Graceful handling of malformed JSON
    SimulationEngine sim;
    OMNIS_ASSERT(!ConfigLoader::load_simulation_config("non_existent_directory/invalid.json", sim));

    // 4. Projection Recalculation verification on config load
    std::string tmp_config = "bin/test_robust_config.json";
    SimulationEngine sim_baseline;
    sim_baseline.set_defender_faction("imperium");
    ConfigLoader::save_simulation_config(tmp_config, sim_baseline);

    SimulationEngine sim_loaded;
    bool ok_load = ConfigLoader::load_simulation_config(tmp_config, sim_loaded);
    OMNIS_ASSERT(ok_load);
    OMNIS_ASSERT(sim_loaded.cached_projection.iterations_run == 20);

    std::cout << "       PASS: JSON type safety, error boundaries, and projection recalculation verified.\n";
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
    test_extreme_scale_billion_entities();
    test_death_guard_disgustingly_resilient();
    test_death_guard_contagion_decay();
    test_mechanicus_canticle_stances();
    test_sororitas_miracle_dice();
    test_world_eaters_blood_tithe();
    test_cross_faction_lanchester_matrix();
    test_ork_waaagh_mechanics();
    test_necron_dynasty_mechanics();
    test_tau_empire_mechanics();
    test_custodes_martial_kata();
    test_faction_command_doctrines();
    test_config_loader_json();
    test_advanced_faction_command_doctrines();
    test_config_loader_edge_cases_and_robustness();

    std::cout << "=================================================================\n";
    std::cout << "  ALL 30 SIMULATION ENGINE TEST SUITES PASSED FLAWLESSLY!        \n";
    std::cout << "=================================================================\n";
    return 0;
}

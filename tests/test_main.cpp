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

int main() {
    std::cout << "=================================================================\n";
    std::cout << "  OMNISBELLI // AUTOMATED COMBAT ENGINE VERIFICATION SUITE       \n";
    std::cout << "=================================================================\n";

    test_unit_mechanics();
    test_lanchester_laws();
    test_excess_damage_cascade();

    std::cout << "=================================================================\n";
    std::cout << "  ALL 3 SIMULATION ENGINE TEST SUITES PASSED FLAWLESSLY!        \n";
    std::cout << "=================================================================\n";
    return 0;
}

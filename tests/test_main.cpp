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

int main() {
    std::cout << "=================================================================\n";
    std::cout << "  OMNISBELLI // AUTOMATED COMBAT ENGINE VERIFICATION SUITE       \n";
    std::cout << "=================================================================\n";

    test_unit_mechanics();

    std::cout << "=================================================================\n";
    std::cout << "  ALL 1 SIMULATION ENGINE TEST SUITES PASSED FLAWLESSLY!        \n";
    std::cout << "=================================================================\n";
    return 0;
}

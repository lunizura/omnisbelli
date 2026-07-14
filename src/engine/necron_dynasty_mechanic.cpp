#include "engine/necron_dynasty_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

NecronDynastyMechanic::NecronDynastyMechanic()
    : reanimation_efficiency(0.20),
      total_reanimated_units(0.0),
      living_metal_hp_repaired(0.0),
      pending_reanimation_pool(0.0),
      overcharge_active(false),
      overcharge_time_remaining(0.0),
      phase_out_triggered(false) {}

std::string NecronDynastyMechanic::get_status_description() const {
    if (phase_out_triggered) {
        return "CRITICAL CASUALTIES // PHASE-OUT PROTOCOL IMMINENT";
    }
    std::string desc = "REANIMATION: " + std::to_string(static_cast<int>(total_reanimated_units)) + " RISEN";
    if (overcharge_active) {
        desc += " [OVERCHARGE " + std::to_string(static_cast<int>(overcharge_time_remaining)) + "s]";
    } else {
        desc += " // LIVING METAL ACTIVE";
    }
    return desc;
}

std::string NecronDynastyMechanic::get_telemetry_tag() const {
    if (phase_out_triggered) {
        return "PHASE-OUT";
    }
    if (overcharge_active) {
        return "REANIM: BOOST";
    }
    return "REANIM: NOMINAL";
}

double NecronDynastyMechanic::get_telemetry_ratio() const {
    if (overcharge_active) {
        return std::clamp(overcharge_time_remaining / 30.0, 0.0, 1.0);
    }
    return 0.85; // High constant cybernetic readiness
}

void NecronDynastyMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)enemy;
    (void)active_sector;
    if (friendly.is_defeated() || dt <= 0.0) return;

    // Check Phase-Out condition: if army drops below 25% initial strength
    double survival_pct = friendly.get_survival_percentage();
    phase_out_triggered = (survival_pct > 0.0 && survival_pct < 25.0);

    // Update overcharge timer
    if (overcharge_active) {
        overcharge_time_remaining -= dt;
        if (overcharge_time_remaining <= 0.0) {
            overcharge_active = false;
            reanimation_efficiency = 0.20;
        }
    }

    // 1. Reanimation Protocols: reconstitute fallen warriors each tick
    if (pending_reanimation_pool > 0.0) {
        // Reconstitute up to 25,000 units per second from the pool (60,000 during overcharge)
        double rate_per_sec = overcharge_active ? 60000.0 : 25000.0;
        double to_revive = std::min(pending_reanimation_pool, rate_per_sec * dt);
        double actual_revived = 0.0;

        // Apply revived units to necron_warrior first, then immortals
        UnitClass* warriors = friendly.find_unit("necron_warrior");
        if (warriors && warriors->count < warriors->initial_count) {
            double deficit = warriors->initial_count - warriors->count;
            double added = std::min(to_revive, deficit);
            warriors->count += added;
            total_reanimated_units += added;
            actual_revived += added;
            to_revive -= added;
        }

        if (to_revive > 0.0) {
            UnitClass* immortals = friendly.find_unit("immortals");
            if (immortals && immortals->count < immortals->initial_count) {
                double deficit = immortals->initial_count - immortals->count;
                double added = std::min(to_revive, deficit);
                immortals->count += added;
                total_reanimated_units += added;
                actual_revived += added;
            }
        }

        pending_reanimation_pool = std::max(0.0, pending_reanimation_pool - actual_revived);
    }

    // 2. Living Metal Hull Regeneration for Monoliths and vehicles
    UnitClass* monolith = friendly.find_unit("monolith");
    if (monolith && monolith->is_alive() && monolith->count < monolith->initial_count) {
        // Self-repairs up to 0.5% max count per second
        double repair_rate = monolith->initial_count * 0.005 * dt;
        double deficit = monolith->initial_count - monolith->count;
        double repaired = std::min(deficit, repair_rate);
        monolith->count += repaired;
        living_metal_hp_repaired += repaired * monolith->hp_per_unit;
    }
}

void NecronDynastyMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    // Gauss weaponry flays molecular bonds: innate +0.12 armor penetration
    out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.12);
}

double NecronDynastyMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    // Living metal chassis disperses kinetic and energy shock: 10% passive mitigation
    return incoming_raw_damage * 0.90;
}

void NecronDynastyMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    (void)enemy;
    if (event.units_killed <= 0.0) return;

    if (event.is_friendly) {
        // Reanimation Protocols queue: reanimation_efficiency fraction enters resurrection pool
        double eligible_reanimations = event.units_killed * reanimation_efficiency;
        pending_reanimation_pool += eligible_reanimations;
    }
}

bool NecronDynastyMechanic::trigger_resurrection_overcharge() {
    overcharge_active = true;
    overcharge_time_remaining = 30.0;
    reanimation_efficiency = 0.50; // 50% reanimation rate
    return true;
}

void NecronDynastyMechanic::reset() {
    reanimation_efficiency = 0.20;
    total_reanimated_units = 0.0;
    living_metal_hp_repaired = 0.0;
    pending_reanimation_pool = 0.0;
    overcharge_active = false;
    overcharge_time_remaining = 0.0;
    phase_out_triggered = false;
}

} // namespace omnisbelli

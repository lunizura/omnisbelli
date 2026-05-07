#include "engine/lanchester.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace omnisbelli {

void LanchesterCombat::compute_combat_tick(
    ArmyRoster& defenders,
    ArmyRoster& attackers,
    Sector& active_sector,
    LogisticsSystem& logistics,
    SynapseNetwork& synapse,
    double dt,
    CombatStepTelemetry& telemetry
) {
    if (dt <= 0.0) return;

    telemetry = CombatStepTelemetry{0, 0, 0, 0, 1.0, 1.0, 0, 0, 0};

    if (defenders.is_defeated() || attackers.is_defeated() || active_sector.is_fallen()) {
        return;
    }

    // 1. Update logistics and synapse web
    logistics.update(dt);
    synapse.update(attackers, dt);

    telemetry.synapse_ratio = synapse.synapse_ratio;

    // 2. Swarm frontage & Lanchester Chokepoint physics
    double swarm_front_footprint = attackers.get_total_frontline_footprint();
    double choke_ratio = active_sector.get_chokepoint_ratio(swarm_front_footprint);
    telemetry.chokepoint_ratio = choke_ratio;

    // 3. Compute Defender Firepower
    double total_defender_dps_raw = 0.0;
    double total_defender_dps_eff = 0.0;

    // Density factor for splash weapons (artillery/titans deal massive AoE if swarm is crowded in choke)
    double swarm_density = (active_sector.chokepoint_width > 0.0)
        ? std::min(4.0, swarm_front_footprint / active_sector.chokepoint_width)
        : 1.0;

    // Buckets for defender damage by target priority
    double damage_priority_synapse = 0.0;
    double damage_priority_heavy = 0.0;
    double damage_priority_swarm = 0.0;
    double damage_priority_balanced = 0.0;

    for (auto& unit : defenders.units) {
        if (!unit.is_alive()) continue;

        double unit_raw = unit.count * unit.dps_per_unit;
        total_defender_dps_raw += unit_raw;

        // Logistics ammo consumption & efficiency
        double ammo_need = unit.ammo_consumption_per_sec * unit.count * dt;
        double ammo_eff = (unit.ammo_type != AmmoType::AMMO_NONE)
            ? logistics.consume_ammo(unit.ammo_type, ammo_need)
            : 1.0;

        // Splash efficiency against dense swarms
        double splash_mult = 1.0;
        if (unit.splash_factor > 1.0 && swarm_density > 0.5) {
            splash_mult = 1.0 + (unit.splash_factor - 1.0) * (swarm_density / 2.0);
        }

        double unit_eff_dps = unit_raw * ammo_eff * splash_mult;
        total_defender_dps_eff += unit_eff_dps;

        double unit_damage_step = unit_eff_dps * dt;
        unit.total_damage_dealt += unit_damage_step;

        switch (unit.target_priority) {
        case TargetPriority::PRIORITY_SYNAPSE:
            damage_priority_synapse += unit_damage_step;
            break;
        case TargetPriority::PRIORITY_HEAVY:
            damage_priority_heavy += unit_damage_step;
            break;
        case TargetPriority::PRIORITY_SWARM:
            damage_priority_swarm += unit_damage_step;
            break;
        case TargetPriority::PRIORITY_BALANCED:
        default:
            damage_priority_balanced += unit_damage_step;
            break;
        }
    }

    telemetry.defender_dps_raw = total_defender_dps_raw;
    telemetry.defender_dps_effective = total_defender_dps_eff;

    // Apply Synapse vulnerability multiplier (Instinctive behavior takes 2.0x casualties)
    double synapse_vuln = synapse.get_casualty_multiplier();
    damage_priority_synapse *= synapse_vuln;
    damage_priority_heavy *= synapse_vuln;
    damage_priority_swarm *= synapse_vuln;
    damage_priority_balanced *= synapse_vuln;

    // 4. Allocate Defender Damage onto Attacker Roster
    double total_attacker_killed_step = 0.0;

    // Helper to distribute damage pool across a list of unit pointers
    auto distribute_pool = [&](double pool, const std::vector<UnitClass*>& targets) -> double {
        if (pool <= 0.0 || targets.empty()) return pool;
        double total_target_hp = 0.0;
        for (auto* t : targets) {
            if (t->is_alive()) total_target_hp += t->get_total_hp();
        }
        if (total_target_hp <= 0.0) return pool;

        double remaining_pool = pool;
        for (auto* t : targets) {
            if (!t->is_alive()) continue;
            double share = pool * (t->get_total_hp() / total_target_hp);
            double effective_armor = std::clamp(t->armor, 0.0, 0.90);
            double killed = t->take_damage(share);
            total_attacker_killed_step += killed;

            double mitigated_used = killed * t->hp_per_unit;
            double raw_used = (1.0 - effective_armor > 0.0)
                ? (mitigated_used / (1.0 - effective_armor))
                : share;
            remaining_pool -= std::min(share, raw_used);
        }
        return std::max(0.0, remaining_pool);
    };

    // Classify attacker units
    std::vector<UnitClass*> synapse_targets;
    std::vector<UnitClass*> heavy_targets;
    std::vector<UnitClass*> swarm_targets;
    std::vector<UnitClass*> all_targets;

    for (auto& u : attackers.units) {
        if (!u.is_alive()) continue;
        all_targets.push_back(&u);
        if (u.is_synapse) {
            synapse_targets.push_back(&u);
        }
        if (u.category == UnitCategory::ARMOR_VEHICLE || u.category == UnitCategory::SUPER_HEAVY_TITAN) {
            heavy_targets.push_back(&u);
        }
        if (u.category == UnitCategory::INFANTRY) {
            swarm_targets.push_back(&u);
        }
    }

    // Apply prioritized damage pools (with fallback to all_targets if specific bucket exhausted)
    double leftover = distribute_pool(damage_priority_synapse, synapse_targets);
    damage_priority_balanced += leftover;

    leftover = distribute_pool(damage_priority_heavy, heavy_targets);
    damage_priority_balanced += leftover;

    leftover = distribute_pool(damage_priority_swarm, swarm_targets);
    damage_priority_balanced += leftover;

    distribute_pool(damage_priority_balanced, all_targets);

    telemetry.attacker_casualties_step = total_attacker_killed_step;

    // 5. Compute Attacker Firepower
    // Lanchester's Linear Law bottleneck: only frontline organisms within chokepoint reach defenders
    // Synapse modifier: Instinctive behavior drops attack by 70%
    double synapse_att_mult = synapse.get_attack_multiplier();
    double total_attacker_dps_raw = 0.0;
    double total_attacker_dps_eff = 0.0;
    double siege_wall_dps = 0.0;

    for (auto& unit : attackers.units) {
        if (!unit.is_alive()) continue;

        double unit_raw = unit.count * unit.dps_per_unit;
        total_attacker_dps_raw += unit_raw;

        // Frontline units are constrained by chokepoint ratio
        // Backline/artillery units (e.g. spore mines/biovores) are less constrained
        double choke_factor = (unit.backline_ratio > 0.5) ? std::max(0.5, choke_ratio) : choke_ratio;
        double unit_eff = unit_raw * choke_factor * synapse_att_mult;
        total_attacker_dps_eff += unit_eff;

        unit.total_damage_dealt += unit_eff * dt;

        // Carnifexes and Bio-Titans batter the fortifications
        if (unit.category == UnitCategory::ARMOR_VEHICLE || unit.category == UnitCategory::SUPER_HEAVY_TITAN) {
            siege_wall_dps += unit_eff * 0.15;
        }
    }

    telemetry.attacker_dps_raw = total_attacker_dps_raw;
    telemetry.attacker_dps_effective = total_attacker_dps_eff;

    // 6. Structural Siege Damage to Active Sector Wall
    // Heavy siege units batter the fortifications directly, while general assault forces chip away
    double wall_dps = siege_wall_dps * 2.5 + total_attacker_dps_eff * 0.15;
    double wall_damage = wall_dps * dt;
    active_sector.apply_siege_damage(wall_damage);
    telemetry.sector_breach_step = active_sector.breach_percentage;

    // 7. Allocate Attacker Damage onto Defenders Behind Fortifications
    // Fortifications protect the garrison. Casualties scale with breach percentage.
    double breach_ratio = active_sector.breach_percentage / 100.0;
    double cover = active_sector.fortification_cover;
    double exposure = 1.0;
    if (!active_sector.is_fallen()) {
        if (active_sector.designation == "SEC-ORBIT") {
            // Orbital platforms take the bombardment; surface garrison has minimal exposure
            exposure = 0.002 + 0.02 * breach_ratio;
        } else {
            // Quadratic exposure: low breach means defenders are safe in bunkers; breach opens direct fire
            double penetration = 0.04 + 0.96 * (breach_ratio * breach_ratio);
            exposure = (1.0 - cover) * penetration;
        }
    }
    double total_incoming_defender_damage = total_attacker_dps_eff * dt * exposure;

    // Frontline defenders absorb 85% of incoming fire; backline artillery absorbs only 15%
    double front_pool = total_incoming_defender_damage * 0.85;
    double back_pool = total_incoming_defender_damage * 0.15;

    std::vector<UnitClass*> front_defenders;
    std::vector<UnitClass*> back_defenders;

    for (auto& u : defenders.units) {
        if (!u.is_alive()) continue;
        if (u.backline_ratio >= 0.5) {
            back_defenders.push_back(&u);
        } else {
            front_defenders.push_back(&u);
        }
    }

    double total_defender_killed_step = 0.0;

    auto kill_defenders = [&](double pool, const std::vector<UnitClass*>& group) -> double {
        if (pool <= 0.0 || group.empty()) return pool;
        double total_hp = 0.0;
        for (auto* d : group) {
            if (d->is_alive()) total_hp += d->get_total_hp();
        }
        if (total_hp <= 0.0) return pool;

        double left = pool;
        for (auto* d : group) {
            if (!d->is_alive()) continue;
            double share = pool * (d->get_total_hp() / total_hp);
            double effective_armor = std::clamp(d->armor, 0.0, 0.90);
            double killed = d->take_damage(share);
            total_defender_killed_step += killed;

            double mitigated_used = killed * d->hp_per_unit;
            double raw_used = (1.0 - effective_armor > 0.0)
                ? (mitigated_used / (1.0 - effective_armor))
                : share;
            left -= std::min(share, raw_used);
        }
        return std::max(0.0, left);
    };

    double leftover_front = kill_defenders(front_pool, front_defenders);
    back_pool += leftover_front;
    kill_defenders(back_pool, back_defenders);

    telemetry.defender_casualties_step = total_defender_killed_step;
}

} // namespace omnisbelli

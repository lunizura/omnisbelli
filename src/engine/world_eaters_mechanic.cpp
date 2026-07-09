#include "engine/world_eaters_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

WorldEatersMechanic::WorldEatersMechanic()
    : blood_tithe_points(0.0),
      total_battlefield_deaths(0.0),
      tithe_tier(0),
      bloodthirsters_summoned(0.0),
      spiteful_demise_damage_dealt(0.0) {}

std::string WorldEatersMechanic::get_status_description() const {
    std::string desc = "BLOOD TITHE: " + std::to_string(static_cast<int>(blood_tithe_points)) + " PTS [TIER " + std::to_string(tithe_tier) + "]";
    if (tithe_tier >= 4) {
        desc += " // BLOODTHIRSTER APOTHEOSIS ACTIVE";
    } else if (tithe_tier >= 3) {
        desc += " // TOTAL CARNAGE [+35% MELEE DPS]";
    } else if (tithe_tier >= 2) {
        desc += " // WARP BLADES [+15% AP]";
    } else if (tithe_tier >= 1) {
        desc += " // SPITEFUL DEMISE RETALIATION";
    }
    return desc;
}

std::string WorldEatersMechanic::get_telemetry_tag() const {
    return "TITHE: T" + std::to_string(tithe_tier) + " (" + std::to_string(static_cast<int>(blood_tithe_points)) + "P)";
}

double WorldEatersMechanic::get_telemetry_ratio() const {
    return std::clamp(blood_tithe_points / 100.0, 0.0, 1.0);
}

void WorldEatersMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)enemy;
    (void)active_sector;
    (void)dt;
    if (friendly.is_defeated()) return;

    // Determine Tithe Tier
    if (blood_tithe_points >= 100.0) {
        tithe_tier = 4;
        // Bloodthirster summoning
        if (bloodthirsters_summoned < 20.0) {
            UnitClass* bt = friendly.find_unit("bloodthirster_avatar");
            if (bt) {
                bt->count = std::min(bt->initial_count, bt->count + 5.0);
                bloodthirsters_summoned += 5.0;
            }
        }
    } else if (blood_tithe_points >= 50.0) {
        tithe_tier = 3;
    } else if (blood_tithe_points >= 25.0) {
        tithe_tier = 2;
    } else if (blood_tithe_points >= 10.0) {
        tithe_tier = 1;
    } else {
        tithe_tier = 0;
    }
}

void WorldEatersMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    // Tier 2: Warp Blades grants bonus armor penetration
    if (tithe_tier >= 2) {
        out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.15);
    }
    // Tier 3: Total Carnage grants +35% attack rate
    if (tithe_tier >= 3) {
        out_damage.raw_damage *= 1.35;
    }
    // Tier 4: Apotheosis frenzy
    if (tithe_tier >= 4) {
        out_damage.raw_damage *= 1.25;
    }
}

double WorldEatersMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    // Khorne cares not from whence the blood flows; no cowardly shielding, but rage ignores morale
    return incoming_raw_damage;
}

void WorldEatersMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    if (event.units_killed <= 0.0) return;

    total_battlefield_deaths += event.units_killed;

    // Both friendly and enemy casualties feed the Blood Tithe (80,000 kills = 1 Tithe Point)
    double points_gained = event.units_killed / 80000.0;
    blood_tithe_points = std::min(120.0, blood_tithe_points + points_gained);

    // Tier 1: Spiteful Demise (Fallen Berserkers explode upon death dealing retaliation damage)
    if (tithe_tier >= 1 && event.is_friendly) {
        double retal_damage = event.units_killed * 12.0; // 12 damage per dead berserker
        spiteful_demise_damage_dealt += retal_damage;
        for (auto& u : enemy.units) {
            if (u.backline_ratio <= 0.5 && u.is_alive()) {
                u.take_damage(retal_damage * 0.35);
                break;
            }
        }
    }
}

void WorldEatersMechanic::reset() {
    blood_tithe_points = 0.0;
    total_battlefield_deaths = 0.0;
    tithe_tier = 0;
    bloodthirsters_summoned = 0.0;
    spiteful_demise_damage_dealt = 0.0;
}

} // namespace omnisbelli

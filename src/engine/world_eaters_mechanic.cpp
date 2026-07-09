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
    return "BLOOD TITHE: " + std::to_string(static_cast<int>(blood_tithe_points)) + " PTS [TIER " + std::to_string(tithe_tier) + "]";
}

std::string WorldEatersMechanic::get_telemetry_tag() const {
    return "TITHE: T" + std::to_string(tithe_tier);
}

double WorldEatersMechanic::get_telemetry_ratio() const {
    return std::clamp(blood_tithe_points / 100.0, 0.0, 1.0);
}

void WorldEatersMechanic::pre_combat_tick(ArmyRoster& friendly, ArmyRoster& enemy, Sector& active_sector, double dt) {
    (void)friendly; (void)enemy; (void)active_sector; (void)dt;
}

void WorldEatersMechanic::modify_outgoing_damage(const ArmyRoster& friendly, DamagePacket& out_damage, double dt) {
    (void)friendly; (void)out_damage; (void)dt;
}

double WorldEatersMechanic::apply_incoming_mitigation(ArmyRoster& friendly, double incoming_raw_damage, double dt) {
    (void)friendly; (void)dt;
    return incoming_raw_damage;
}

void WorldEatersMechanic::on_casualty_event(ArmyRoster& friendly, ArmyRoster& enemy, const CasualtyEvent& event) {
    (void)friendly;
    if (event.units_killed <= 0.0) return;
    total_battlefield_deaths += event.units_killed;
    double points_gained = event.units_killed / 80000.0;
    blood_tithe_points = std::min(120.0, blood_tithe_points + points_gained);
    if (tithe_tier >= 1 && event.is_friendly) {
        double retal_damage = event.units_killed * 12.0;
        spiteful_demise_damage_dealt += retal_damage;
        for (auto& u : enemy.units) {
            if (u.backline_ratio <= 0.5 && u.is_alive()) {
                u.take_damage(retal_damage * 0.35);
                break;
            }
        }
    }
}

void WorldEatersMechanic::reset() {}

} // namespace omnisbelli

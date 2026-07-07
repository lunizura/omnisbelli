#include "engine/sororitas_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

SororitasMechanic::SororitasMechanic()
    : miracle_points(15.0),
      total_miracle_points_generated(15.0),
      casualties_since_last_dice(0.0),
      divine_shield_active(false),
      divine_shield_timer(0.0),
      damage_absorbed_by_faith(0.0),
      saint_resurrected(false) {}

std::string SororitasMechanic::get_status_description() const {
    return "ACTS OF FAITH // MIRACLE POOL: " + std::to_string(static_cast<int>(miracle_points)) + "/100";
}

std::string SororitasMechanic::get_telemetry_tag() const {
    return "FAITH: " + std::to_string(static_cast<int>(miracle_points)) + " PTS";
}

double SororitasMechanic::get_telemetry_ratio() const {
    return std::clamp(miracle_points / 100.0, 0.0, 1.0);
}

void SororitasMechanic::pre_combat_tick(ArmyRoster& friendly, ArmyRoster& enemy, Sector& active_sector, double dt) {
    (void)friendly; (void)enemy; (void)active_sector; (void)dt;
}

void SororitasMechanic::modify_outgoing_damage(const ArmyRoster& friendly, DamagePacket& out_damage, double dt) {
    (void)friendly; (void)out_damage; (void)dt;
}

double SororitasMechanic::apply_incoming_mitigation(ArmyRoster& friendly, double incoming_raw_damage, double dt) {
    (void)friendly; (void)dt;
    return incoming_raw_damage;
}

void SororitasMechanic::on_casualty_event(ArmyRoster& friendly, ArmyRoster& enemy, const CasualtyEvent& event) {
    (void)friendly; (void)enemy;
    if (event.is_friendly && event.units_killed > 0.0) {
        casualties_since_last_dice += event.units_killed;
        const double casualty_threshold = 12000.0;
        if (casualties_since_last_dice >= casualty_threshold) {
            double gained = std::floor(casualties_since_last_dice / casualty_threshold);
            casualties_since_last_dice = std::fmod(casualties_since_last_dice, casualty_threshold);
            miracle_points = std::min(100.0, miracle_points + gained);
            total_miracle_points_generated += gained;
        }
    }
}

bool SororitasMechanic::trigger_divine_intervention() { return false; }
void SororitasMechanic::reset() {}

} // namespace omnisbelli

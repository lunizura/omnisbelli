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
    std::string desc = "ACTS OF FAITH // MIRACLE POOL: " + std::to_string(static_cast<int>(miracle_points)) + "/100";
    if (divine_shield_active) {
        desc += " [SHIELD OF FAITH 4++ ENGAGED]";
    } else if (miracle_points >= 50.0) {
        desc += " [HOLY TRINITY PURITY +35% DPS]";
    }
    return desc;
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
void SororitasMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    // Holy Trinity Purity bonus when faith pool is abundant
    if (miracle_points >= 50.0) {
        double faith_ratio = (miracle_points - 50.0) / 50.0;
        out_damage.raw_damage *= (1.0 + 0.35 * faith_ratio);
        out_damage.armor_penetration = std::min(0.85, out_damage.armor_penetration + 0.15);
    }
}

double SororitasMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)dt;
    if (incoming_raw_damage <= 0.0) return 0.0;

    // Active Shield of Faith 4++ invulnerability
    if (divine_shield_active) {
        double absorbed = incoming_raw_damage * 0.50; // Halves all incoming damage
        damage_absorbed_by_faith += absorbed;
        return incoming_raw_damage - absorbed;
    }

    // Auto-proc Divine Shield if sudden burst exceeds 40% of living HP and we have >= 15 faith
    double living_hp = friendly.get_total_living_hp();
    if (incoming_raw_damage > living_hp * 0.35 && miracle_points >= 15.0) {
        miracle_points -= 15.0;
        divine_shield_active = true;
        divine_shield_timer = 6.0;
        double absorbed = incoming_raw_damage * 0.50;
        damage_absorbed_by_faith += absorbed;
        return incoming_raw_damage - absorbed;
    }

    return incoming_raw_damage;
}

void SororitasMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    (void)enemy;
    // Friendly casualties convert directly into Martyrdom Miracle Points
    if (event.is_friendly && event.units_killed > 0.0) {
        casualties_since_last_dice += event.units_killed;
        const double casualty_threshold = 12000.0; // 12,000 casualties per faith point
        if (casualties_since_last_dice >= casualty_threshold) {
            double gained = std::floor(casualties_since_last_dice / casualty_threshold);
            casualties_since_last_dice = std::fmod(casualties_since_last_dice, casualty_threshold);
            miracle_points = std::min(100.0, miracle_points + gained);
            total_miracle_points_generated += gained;
        }
    }
}

bool SororitasMechanic::trigger_divine_intervention() {
    if (miracle_points < 25.0) return false;
    miracle_points -= 25.0;
    divine_shield_active = true;
    divine_shield_timer = 12.0;
    return true;
}

void SororitasMechanic::reset() {
    miracle_points = 15.0;
    total_miracle_points_generated = 15.0;
    casualties_since_last_dice = 0.0;
    divine_shield_active = false;
    divine_shield_timer = 0.0;
    damage_absorbed_by_faith = 0.0;
    saint_resurrected = false;
}

} // namespace omnisbelli

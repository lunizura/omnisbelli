#include "engine/ork_waaagh_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

OrkWaaaghMechanic::OrkWaaaghMechanic()
    : waaagh_momentum(15.0),
      waaagh_frenzy_active(false),
      frenzy_time_remaining(0.0),
      dakkadakka_bonus_kills(0.0),
      grot_shields_absorbed_damage(0.0),
      total_ork_casualties_suffered(0.0) {}

std::string OrkWaaaghMechanic::get_status_description() const {
    if (waaagh_frenzy_active) {
        return "WAAAGH! FRENZY ACTIVE [" + std::to_string(static_cast<int>(frenzy_time_remaining)) + "s] // +40% MELEE + DAKKADAKKA!";
    }
    std::string desc = "WAAAGH! MOMENTUM: " + std::to_string(static_cast<int>(waaagh_momentum)) + "%";
    if (waaagh_momentum >= 80.0) {
        desc += " // FRENZY CRITICAL THRESHOLD";
    } else if (waaagh_momentum >= 50.0) {
        desc += " // HIGH GREEN MOB CHARGE";
    } else {
        desc += " // MOB GATHERING SPEED";
    }
    return desc;
}

std::string OrkWaaaghMechanic::get_telemetry_tag() const {
    if (waaagh_frenzy_active) {
        return "WAAAGH: FRENZY";
    }
    return "WAAAGH: " + std::to_string(static_cast<int>(waaagh_momentum)) + "%";
}

double OrkWaaaghMechanic::get_telemetry_ratio() const {
    return std::clamp(waaagh_momentum / 100.0, 0.0, 1.0);
}

void OrkWaaaghMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)enemy;
    (void)active_sector;
    if (friendly.is_defeated() || dt <= 0.0) return;

    if (waaagh_frenzy_active) {
        frenzy_time_remaining -= dt;
        if (frenzy_time_remaining <= 0.0) {
            waaagh_frenzy_active = false;
            waaagh_momentum = 25.0; // drops back after frenzy
        }
    } else {
        // Natural slight decay if not triggered
        waaagh_momentum = std::max(10.0, waaagh_momentum - (0.4 * dt));
        // Auto-trigger when reaching 100%
        if (waaagh_momentum >= 100.0) {
            trigger_waaagh_roar();
        }
    }
}

void OrkWaaaghMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;

    // 1. Dakkadakka burst hits: exploding 6s give ~20% bonus burst fire and +0.08 AP
    out_damage.raw_damage *= 1.20;
    out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.08);

    // 2. WAAAGH! Melee frenzy charge boost
    if (waaagh_frenzy_active) {
        out_damage.raw_damage *= 1.40;
        out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.12);
    } else if (waaagh_momentum >= 50.0) {
        // Momentum charge scaling
        double charge_bonus = 1.0 + (waaagh_momentum - 50.0) * 0.005; // up to +25%
        out_damage.raw_damage *= charge_bonus;
    }
}

double OrkWaaaghMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)dt;
    if (friendly.is_defeated() || incoming_raw_damage <= 0.0) return incoming_raw_damage;

    // Grot Shields: Meatshield absorbs 20% of incoming anti-armor fire
    UnitClass* boyz = friendly.find_unit("ork_boyz");
    if (boyz && boyz->is_alive() && boyz->count > 10000.0) {
        double absorbed = incoming_raw_damage * 0.20;
        grot_shields_absorbed_damage += absorbed;
        return incoming_raw_damage * 0.80;
    }

    return incoming_raw_damage;
}

void OrkWaaaghMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    (void)enemy;
    if (event.units_killed <= 0.0) return;

    if (event.is_friendly) {
        total_ork_casualties_suffered += event.units_killed;
    }

    // Both friendly and enemy casualties fuel Ork psychic excitement (60,000 kills = 1% momentum)
    double momentum_gain = event.units_killed / 60000.0;
    waaagh_momentum = std::min(100.0, waaagh_momentum + momentum_gain);

    if (waaagh_momentum >= 100.0 && !waaagh_frenzy_active) {
        trigger_waaagh_roar();
    }
}

bool OrkWaaaghMechanic::trigger_waaagh_roar() {
    waaagh_frenzy_active = true;
    frenzy_time_remaining = 25.0;
    waaagh_momentum = 100.0;
    return true;
}

void OrkWaaaghMechanic::reset() {
    waaagh_momentum = 15.0;
    waaagh_frenzy_active = false;
    frenzy_time_remaining = 0.0;
    dakkadakka_bonus_kills = 0.0;
    grot_shields_absorbed_damage = 0.0;
    total_ork_casualties_suffered = 0.0;
}

} // namespace omnisbelli

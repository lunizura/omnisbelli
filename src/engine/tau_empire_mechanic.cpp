#include "engine/tau_empire_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

TauEmpireMechanic::TauEmpireMechanic()
    : markerlight_tokens(2.0),
      current_stance(TauStance::MONTKA),
      stance_elapsed_time(0.0),
      shield_drone_damage_absorbed(0.0),
      total_seeker_missiles_fired(0.0) {}

std::string TauEmpireMechanic::get_status_description() const {
    std::string desc = "MARKERLIGHT: " + std::to_string(static_cast<int>(markerlight_tokens)) + "/10 TOKENS // ";
    if (current_stance == TauStance::MONTKA) {
        if (stance_elapsed_time <= 90.0) {
            desc += "MONT'KA KILLING BLOW ACTIVE [+" + std::to_string(static_cast<int>(90.0 - stance_elapsed_time)) + "s]";
        } else {
            desc += "MONT'KA FADED // REPOSITION REQUIRED";
        }
    } else {
        double kauyon_bonus = std::min(40.0, stance_elapsed_time * 0.5);
        desc += "KAUYON PATIENT HUNTER [+" + std::to_string(static_cast<int>(kauyon_bonus)) + "% SUSTAINED DPS]";
    }
    return desc;
}

std::string TauEmpireMechanic::get_telemetry_tag() const {
    std::string tag = (current_stance == TauStance::MONTKA) ? "MONT'KA" : "KAUYON";
    return tag + " (ML:" + std::to_string(static_cast<int>(markerlight_tokens)) + ")";
}

double TauEmpireMechanic::get_telemetry_ratio() const {
    return std::clamp(markerlight_tokens / 10.0, 0.0, 1.0);
}

void TauEmpireMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)enemy;
    (void)active_sector;
    if (friendly.is_defeated() || dt <= 0.0) return;

    stance_elapsed_time += dt;

    // Pathfinder drones paint targets: accumulates markerlight tokens over time
    UnitClass* suits = friendly.find_unit("crisis_battlesuits");
    double suits_factor = (suits && suits->is_alive()) ? 1.5 : 1.0;
    markerlight_tokens = std::min(10.0, markerlight_tokens + (0.25 * suits_factor * dt));
}

void TauEmpireMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;

    // 1. Markerlight Progressive Target Lock
    if (markerlight_tokens >= 9.0) {
        // Tier 3: Seeker missile mass lock
        out_damage.raw_damage *= 1.30;
        out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.25);
        out_damage.ignores_cover = true;
    } else if (markerlight_tokens >= 6.0) {
        // Tier 2: Heavy target painting
        out_damage.raw_damage *= 1.15;
        out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.15);
        out_damage.ignores_cover = true;
    } else if (markerlight_tokens >= 3.0) {
        // Tier 1: Target designated
        out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.08);
    }

    // 2. Tactical Stance Modifiers
    if (current_stance == TauStance::MONTKA) {
        // Mont'ka: Blitz in first 90 seconds
        if (stance_elapsed_time <= 90.0) {
            out_damage.raw_damage *= 1.35;
            out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + 0.10);
        }
    } else {
        // Kauyon: Scales with battle duration (+0.5% DPS per second up to +40%)
        double kauyon_mult = 1.0 + std::min(0.40, stance_elapsed_time * 0.005);
        out_damage.raw_damage *= kauyon_mult;
    }
}

double TauEmpireMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)dt;
    if (friendly.is_defeated() || incoming_raw_damage <= 0.0) return incoming_raw_damage;

    // Shield Drone interception: absorbs 18% of all incoming damage
    double absorbed = incoming_raw_damage * 0.18;
    shield_drone_damage_absorbed += absorbed;
    double mitigated = incoming_raw_damage * 0.82;

    // Kauyon patient defense posture adds another 10% mitigation
    if (current_stance == TauStance::KAUYON) {
        mitigated *= 0.90;
    }

    return mitigated;
}

void TauEmpireMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    (void)enemy;
    (void)event;
    // T'au markerlights degrade slightly on heavy friendly losses
    if (event.is_friendly && event.units_killed > 50000.0) {
        markerlight_tokens = std::max(0.0, markerlight_tokens - 1.0);
    }
}

void TauEmpireMechanic::set_stance(TauStance stance) {
    current_stance = stance;
    stance_elapsed_time = 0.0;
}

void TauEmpireMechanic::add_markerlight_tokens(double amount) {
    markerlight_tokens = std::clamp(markerlight_tokens + amount, 0.0, 10.0);
}

void TauEmpireMechanic::reset() {
    markerlight_tokens = 2.0;
    current_stance = TauStance::MONTKA;
    stance_elapsed_time = 0.0;
    shield_drone_damage_absorbed = 0.0;
    total_seeker_missiles_fired = 0.0;
}

} // namespace omnisbelli

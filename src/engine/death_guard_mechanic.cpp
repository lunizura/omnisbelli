#include "engine/death_guard_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

DeathGuardMechanic::DeathGuardMechanic()
    : contagion_intensity(0.0),
      contested_seconds_in_sector(0.0),
      disgusting_resilience_damage_negated(0.0),
      poxwalkers_spawned(0.0),
      sector_rot_corrosion_pct(0.0) {}

std::string DeathGuardMechanic::get_status_description() const {
    int pct = static_cast<int>(contagion_intensity * 100.0);
    return "NURGLE ROT CONTAGION: " + std::to_string(pct) + "% // DISGUSTINGLY RESILIENT [FNP 5+++]";
}

std::string DeathGuardMechanic::get_telemetry_tag() const {
    int pct = static_cast<int>(contagion_intensity * 100.0);
    return "CONTAGION: " + std::to_string(pct) + "%";
}

double DeathGuardMechanic::get_telemetry_ratio() const {
    return std::clamp(contagion_intensity, 0.0, 1.0);
}

double DeathGuardMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    if (incoming_raw_damage <= 0.0) return 0.0;

    // Disgustingly Resilient (FNP): Negates 33.3% of lethal damage
    const double fnp_factor = 0.50; // D_eff = D_raw / (1 + 0.5) = 0.667 * D_raw
    double mitigated_damage = incoming_raw_damage / (1.0 + fnp_factor);
    disgusting_resilience_damage_negated += (incoming_raw_damage - mitigated_damage);
    return mitigated_damage;
}

void DeathGuardMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)friendly;
    (void)enemy;
    (void)active_sector;
    (void)dt;
}

void DeathGuardMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)out_damage;
    (void)dt;
}

void DeathGuardMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    (void)enemy;
    (void)event;
}

void DeathGuardMechanic::reset() {
    contagion_intensity = 0.0;
    contested_seconds_in_sector = 0.0;
    disgusting_resilience_damage_negated = 0.0;
    poxwalkers_spawned = 0.0;
    sector_rot_corrosion_pct = 0.0;
}

} // namespace omnisbelli

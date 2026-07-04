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

void DeathGuardMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    if (dt <= 0.0 || friendly.is_defeated()) return;

    contested_seconds_in_sector += dt;

    double survival_ratio = friendly.get_total_initial_count() > 0.0
        ? (friendly.get_total_count() / friendly.get_total_initial_count())
        : 0.0;

    // Contagion builds as battle rages in sector
    contagion_intensity = std::min(
        1.0,
        (contested_seconds_in_sector / 240.0) + (0.35 * survival_ratio)
    );

    // Corrode active sector fortification cover
    if (active_sector.fortification_cover > 0.10) {
        double corrosion = 0.002 * contagion_intensity * dt;
        active_sector.fortification_cover = std::max(0.10, active_sector.fortification_cover - corrosion);
        sector_rot_corrosion_pct += corrosion * 100.0;
    }

    // Passive environmental rot attrition on unsealed enemy infantry
    if (contagion_intensity >= 0.20) {
        double rot_rate = 0.0006 * contagion_intensity * dt;
        for (auto& u : enemy.units) {
            // Unsealed infantry units suffer continuous respiratory necrosis
            if (u.category == UnitCategory::INFANTRY &&
                u.id.find("marine") == std::string::npos &&
                u.id.find("custodes") == std::string::npos &&
                u.id.find("necron") == std::string::npos) {
                u.apply_direct_casualties(u.count * rot_rate);
            }
        }
    }
}

void DeathGuardMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    // Contagion corrodes enemy armor, effectively granting additional armor penetration
    out_damage.armor_penetration = std::min(0.85, out_damage.armor_penetration + (0.25 * contagion_intensity));
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

#include "engine/mechanicus_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>

namespace omnisbelli {

MechanicusMechanic::MechanicusMechanic()
    : active_protocol(CanticleProtocol::PROTECTOR_IMPERATIVE),
      protocol_timer(40.0),
      binaric_power_level(100.0),
      radiation_rads_accumulated(0.0),
      electro_arcs_kills(0.0) {}

std::string MechanicusMechanic::get_status_description() const {
    switch (active_protocol) {
    case CanticleProtocol::PROTECTOR_IMPERATIVE:
        return "CANTICLE: PROTECTOR IMPERATIVE [+35% RANGED BALLISTICS / -20% MOBILITY]";
    case CanticleProtocol::CONQUEROR_IMPERATIVE:
        return "CANTICLE: CONQUEROR IMPERATIVE [+45% SHOCK ASSAULT / +10% ARMOR]";
    case CanticleProtocol::SHROUDPSALM_STEALTH:
        return "CANTICLE: SHROUDPSALM [ELECTRO-MAGNETIC CHAFF +35% COVER]";
    case CanticleProtocol::LITANY_ELECTROMANCER:
        return "CANTICLE: LITANY OF ELECTROMANCER [ARC-LIGHTNING CHOKEPOINT DISCHARGE]";
    default:
        return "CANTICLE OF OMNISSIAH ONLINE";
    }
}

std::string MechanicusMechanic::get_telemetry_tag() const {
    switch (active_protocol) {
    case CanticleProtocol::PROTECTOR_IMPERATIVE:
        return "CANTICLE: PROTECTOR";
    case CanticleProtocol::CONQUEROR_IMPERATIVE:
        return "CANTICLE: CONQUEROR";
    case CanticleProtocol::SHROUDPSALM_STEALTH:
        return "CANTICLE: SHROUDPSALM";
    case CanticleProtocol::LITANY_ELECTROMANCER:
        return "CANTICLE: ELECTROMANCER";
    default:
        return "CANTICLE: ACTIVE";
    }
}

double MechanicusMechanic::get_telemetry_ratio() const {
    return std::clamp(protocol_timer / 40.0, 0.0, 1.0);
}

void MechanicusMechanic::cycle_protocol() {
    int next = (static_cast<int>(active_protocol) + 1) % 4;
    active_protocol = static_cast<CanticleProtocol>(next);
    protocol_timer = 40.0;
}

void MechanicusMechanic::set_protocol(CanticleProtocol protocol) {
    active_protocol = protocol;
    protocol_timer = 40.0;
}

void MechanicusMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)friendly;
    if (dt <= 0.0) return;

    // Automatic canticle stanza cycling every 40 seconds
    protocol_timer -= dt;
    if (protocol_timer <= 0.0) {
        cycle_protocol();
    }

    // Rad-saturation permanently suppresses unsealed enemy organic frontline HP
    for (auto& u : enemy.units) {
        if (u.category == UnitCategory::INFANTRY &&
            u.id.find("marine") == std::string::npos &&
            u.id.find("custodes") == std::string::npos &&
            u.id.find("necron") == std::string::npos &&
            u.id.find("titan") == std::string::npos) {
            radiation_rads_accumulated += 0.5 * dt;
            u.hp_per_unit = std::max(5.0, u.hp_per_unit * (1.0 - 0.002 * dt)); // Rad-burn wasting
        }
    }

    // Electromancer Litany: discharges arcs across dense frontline
    if (active_protocol == CanticleProtocol::LITANY_ELECTROMANCER) {
        double arc_dps = 250000.0 * dt;
        for (auto& u : enemy.units) {
            if (u.backline_ratio <= 0.5 && u.is_alive()) {
                double kills = u.take_damage(arc_dps * 0.25);
                electro_arcs_kills += kills;
            }
        }
    }

    // Shroudpsalm cover boost
    if (active_protocol == CanticleProtocol::SHROUDPSALM_STEALTH) {
        active_sector.fortification_cover = std::min(0.90, active_sector.fortification_cover + 0.15 * dt);
    }
}

void MechanicusMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    if (active_protocol == CanticleProtocol::PROTECTOR_IMPERATIVE) {
        out_damage.raw_damage *= 1.35; // +35% ranged precision
        out_damage.armor_penetration = std::min(0.85, out_damage.armor_penetration + 0.10);
    } else if (active_protocol == CanticleProtocol::CONQUEROR_IMPERATIVE) {
        out_damage.raw_damage *= 1.45; // +45% melee shock
    }
}

double MechanicusMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    if (incoming_raw_damage <= 0.0) return 0.0;

    if (active_protocol == CanticleProtocol::SHROUDPSALM_STEALTH) {
        // Electronic cloaking and targeting scattering absorbs 30% of incoming long-range fire
        return incoming_raw_damage * 0.70;
    } else if (active_protocol == CanticleProtocol::CONQUEROR_IMPERATIVE) {
        // Reinforced cybernetic shielding reduces incoming shock by 10%
        return incoming_raw_damage * 0.90;
    }
    return incoming_raw_damage;
}

void MechanicusMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    (void)enemy;
    (void)event;
}

void MechanicusMechanic::reset() {
    active_protocol = CanticleProtocol::PROTECTOR_IMPERATIVE;
    protocol_timer = 40.0;
    binaric_power_level = 100.0;
    radiation_rads_accumulated = 0.0;
    electro_arcs_kills = 0.0;
}

} // namespace omnisbelli

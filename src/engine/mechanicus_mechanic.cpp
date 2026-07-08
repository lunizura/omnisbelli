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
    return "CANTICLE: PROTECTOR IMPERATIVE [+35% RANGED BALLISTICS / -20% MOBILITY]";
}

std::string MechanicusMechanic::get_telemetry_tag() const {
    return "CANTICLE: PROTECTOR";
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

void MechanicusMechanic::pre_combat_tick(ArmyRoster& friendly, ArmyRoster& enemy, Sector& active_sector, double dt) {
    (void)friendly; (void)enemy; (void)active_sector; (void)dt;
}

void MechanicusMechanic::modify_outgoing_damage(const ArmyRoster& friendly, DamagePacket& out_damage, double dt) {
    (void)friendly; (void)dt;
    if (active_protocol == CanticleProtocol::PROTECTOR_IMPERATIVE) {
        out_damage.raw_damage *= 1.35;
        out_damage.armor_penetration = std::min(0.85, out_damage.armor_penetration + 0.10);
    } else if (active_protocol == CanticleProtocol::CONQUEROR_IMPERATIVE) {
        out_damage.raw_damage *= 1.45;
    }
}

double MechanicusMechanic::apply_incoming_mitigation(ArmyRoster& friendly, double incoming_raw_damage, double dt) {
    (void)friendly; (void)dt;
    return incoming_raw_damage;
}

void MechanicusMechanic::on_casualty_event(ArmyRoster& friendly, ArmyRoster& enemy, const CasualtyEvent& event) {
    (void)friendly; (void)enemy; (void)event;
}

void MechanicusMechanic::reset() {}

} // namespace omnisbelli

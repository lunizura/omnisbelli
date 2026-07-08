#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

enum class CanticleProtocol {
    PROTECTOR_IMPERATIVE,     // +35% Ranged DPS, High Accuracy
    CONQUEROR_IMPERATIVE,     // +45% Melee Shock DPS, +10% Armor
    SHROUDPSALM_STEALTH,      // Electro-chaff aura: +35% Defense Cover
    LITANY_ELECTROMANCER      // Arc-lightning discharge on enemy chokepoints
};

class MechanicusMechanic : public IFactionRuleModule {
public:
    CanticleProtocol active_protocol;
    double protocol_timer;
    double binaric_power_level;             // 0.0 to 100.0%
    double radiation_rads_accumulated;
    double electro_arcs_kills;

    MechanicusMechanic();

    std::string get_faction_id() const override { return "adeptus_mechanicus"; }
    std::string get_status_description() const override;
    std::string get_telemetry_tag() const override;
    double get_telemetry_ratio() const override;

    void pre_combat_tick(
        ArmyRoster& friendly,
        ArmyRoster& enemy,
        Sector& active_sector,
        double dt
    ) override;

    void modify_outgoing_damage(
        const ArmyRoster& friendly,
        DamagePacket& out_damage,
        double dt
    ) override;

    double apply_incoming_mitigation(
        ArmyRoster& friendly,
        double incoming_raw_damage,
        double dt
    ) override;

    void on_casualty_event(
        ArmyRoster& friendly,
        ArmyRoster& enemy,
        const CasualtyEvent& event
    ) override;

    void cycle_protocol();
    void set_protocol(CanticleProtocol protocol);

    void reset() override;
};

} // namespace omnisbelli

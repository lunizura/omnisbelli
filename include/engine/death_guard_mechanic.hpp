#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

class DeathGuardMechanic : public IFactionRuleModule {
public:
    double contagion_intensity;             // 0.0 to 1.0
    double contested_seconds_in_sector;
    double disgusting_resilience_damage_negated;
    double poxwalkers_spawned;
    double sector_rot_corrosion_pct;

    DeathGuardMechanic();

    std::string get_faction_id() const override { return "death_guard"; }
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

    void reset() override;
};

} // namespace omnisbelli

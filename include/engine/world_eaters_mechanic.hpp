#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

class WorldEatersMechanic : public IFactionRuleModule {
public:
    double blood_tithe_points;              // 0.0 to 120.0
    double total_battlefield_deaths;
    int tithe_tier;                         // 0 to 4
    double bloodthirsters_summoned;
    double spiteful_demise_damage_dealt;

    WorldEatersMechanic();

    std::string get_faction_id() const override { return "world_eaters"; }
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

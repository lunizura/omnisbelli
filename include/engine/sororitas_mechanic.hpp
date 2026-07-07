#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

class SororitasMechanic : public IFactionRuleModule {
public:
    double miracle_points;                  // 0.0 to 100.0
    double total_miracle_points_generated;
    double casualties_since_last_dice;
    bool divine_shield_active;
    double divine_shield_timer;
    double damage_absorbed_by_faith;
    bool saint_resurrected;

    SororitasMechanic();

    std::string get_faction_id() const override { return "adepta_sororitas"; }
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

    bool trigger_divine_intervention();

    void reset() override;
};

} // namespace omnisbelli

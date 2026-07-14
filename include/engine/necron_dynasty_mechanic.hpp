#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

class NecronDynastyMechanic : public IFactionRuleModule {
public:
    double reanimation_efficiency;          // 0.20 (20% reanimation rate)
    double total_reanimated_units;
    double living_metal_hp_repaired;
    double pending_reanimation_pool;
    bool overcharge_active;
    double overcharge_time_remaining;
    bool phase_out_triggered;

    NecronDynastyMechanic();

    std::string get_faction_id() const override { return "necrons"; }
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

    bool trigger_resurrection_overcharge();
    void reset() override;
};

} // namespace omnisbelli

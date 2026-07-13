#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

class OrkWaaaghMechanic : public IFactionRuleModule {
public:
    double waaagh_momentum;             // 0.0 to 100.0
    bool waaagh_frenzy_active;
    double frenzy_time_remaining;
    double dakkadakka_bonus_kills;
    double grot_shields_absorbed_damage;
    double total_ork_casualties_suffered;

    OrkWaaaghMechanic();

    std::string get_faction_id() const override { return "orks"; }
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

    bool trigger_waaagh_roar();
    void reset() override;
};

} // namespace omnisbelli

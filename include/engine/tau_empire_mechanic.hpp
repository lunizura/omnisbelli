#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

enum class TauStance {
    MONTKA,     // The Killing Blow: aggressive early blitz
    KAUYON      // The Patient Hunter: defensive lure and escalating crossfire
};

class TauEmpireMechanic : public IFactionRuleModule {
public:
    double markerlight_tokens;              // 0.0 to 10.0
    TauStance current_stance;
    double stance_elapsed_time;
    double shield_drone_damage_absorbed;
    double total_seeker_missiles_fired;

    TauEmpireMechanic();

    std::string get_faction_id() const override { return "tau"; }
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

    void set_stance(TauStance stance);
    void add_markerlight_tokens(double amount);
    void reset() override;
};

} // namespace omnisbelli

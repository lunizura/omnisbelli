#pragma once
#include "engine/faction_rule_module.hpp"

namespace omnisbelli {

enum class KataStance {
    KAPTARIS,       // Defensive parry stance: reduces incoming damage
    DACATARAI,      // Sweeping whirlwind stance: maximizes anti-horde DPS
    RENDAX          // Monster and titan-hunter stance: armor-piercing decapitation
};

class CustodesMechanic : public IFactionRuleModule {
public:
    KataStance active_kata;
    double stance_timer;
    double auric_aegis_damage_prevented;
    double rendax_armor_shred;

    CustodesMechanic();

    std::string get_faction_id() const override { return "custodes"; }
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

    void cycle_kata_stance();
    void set_kata_stance(KataStance stance);
    void reset() override;
};

} // namespace omnisbelli

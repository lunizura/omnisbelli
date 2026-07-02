#pragma once
#include <string>
#include <vector>
#include <memory>

namespace omnisbelli {

class ArmyRoster;
class Sector;

struct DamagePacket {
    double raw_damage;
    double armor_penetration; // [0.0, 1.0] additional armor pierce
    double splash_multiplier;
    bool ignores_cover;
};

struct CasualtyEvent {
    double units_killed;
    double hp_lost;
    std::string unit_category;
    bool is_friendly;
};

class IFactionRuleModule {
public:
    virtual ~IFactionRuleModule() = default;

    virtual std::string get_faction_id() const = 0;
    virtual std::string get_status_description() const = 0;
    virtual std::string get_telemetry_tag() const = 0;
    virtual double get_telemetry_ratio() const = 0;

    virtual void pre_combat_tick(
        ArmyRoster& friendly,
        ArmyRoster& enemy,
        Sector& active_sector,
        double dt
    ) = 0;

    virtual void modify_outgoing_damage(
        const ArmyRoster& friendly,
        DamagePacket& out_damage,
        double dt
    ) {
        (void)friendly;
        (void)out_damage;
        (void)dt;
    }

    virtual double apply_incoming_mitigation(
        ArmyRoster& friendly,
        double incoming_raw_damage,
        double dt
    ) {
        (void)friendly;
        (void)dt;
        return incoming_raw_damage;
    }

    virtual void on_casualty_event(
        ArmyRoster& friendly,
        ArmyRoster& enemy,
        const CasualtyEvent& event
    ) {
        (void)friendly;
        (void)enemy;
        (void)event;
    }

    virtual void reset() = 0;
};

} // namespace omnisbelli

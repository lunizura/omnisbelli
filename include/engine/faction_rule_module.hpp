#pragma once
#include <string>
#include <vector>
#include <memory>
#include "engine/synapse_network.hpp"

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
    virtual double get_telemetry_ratio() const = 0; // [0.0, 1.0] for retro progress gauges

    // Pre-combat step: updates internal faction clocks, auras, and resources
    virtual void pre_combat_tick(
        ArmyRoster& friendly,
        ArmyRoster& enemy,
        Sector& active_sector,
        double dt
    ) = 0;

    // Modifies outgoing weapon damage before armor mitigation (e.g. Canticles, Holy Trinity)
    virtual void modify_outgoing_damage(
        const ArmyRoster& friendly,
        DamagePacket& out_damage,
        double dt
    ) {
        (void)friendly;
        (void)out_damage;
        (void)dt;
    }

    // Applies custom damage mitigation (e.g. Disgustingly Resilient, Shield of Faith invulnerability)
    virtual double apply_incoming_mitigation(
        ArmyRoster& friendly,
        double incoming_raw_damage,
        double dt
    ) {
        (void)friendly;
        (void)dt;
        return incoming_raw_damage;
    }

    // Callback on casualties suffered or inflicted (e.g. Miracle points, Blood Tithe, Poxwalker recursion)
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

class GenericFactionModule : public IFactionRuleModule {
public:
    std::string faction_id;
    std::string display_title;

    GenericFactionModule(const std::string& fid, const std::string& title);

    std::string get_faction_id() const override { return faction_id; }
    std::string get_status_description() const override;
    std::string get_telemetry_tag() const override { return faction_id; }
    double get_telemetry_ratio() const override { return 1.0; }

    void pre_combat_tick(
        ArmyRoster& friendly,
        ArmyRoster& enemy,
        Sector& active_sector,
        double dt
    ) override;

    void reset() override {}
};

class TyranidSynapseModule : public IFactionRuleModule {
public:
    SynapseNetwork synapse;

    TyranidSynapseModule() = default;
    explicit TyranidSynapseModule(const SynapseNetwork& net);

    std::string get_faction_id() const override { return "tyranids"; }
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

    void reset() override;
};

// Factory for creating faction rule module by army_id
std::unique_ptr<IFactionRuleModule> create_faction_rule_module(const std::string& faction_id);

} // namespace omnisbelli

#pragma once
#include "engine/unit_class.hpp"
#include <vector>
#include <string>
#include <memory>

namespace omnisbelli {

class ArmyRoster {
public:
    std::string army_id;
    std::string display_name;
    std::string faction_id;
    std::string name;
    std::vector<UnitClass> units;

    ArmyRoster();
    ArmyRoster(const std::string& army_id, const std::string& display_name);

    void add_unit(const UnitClass& unit);
    UnitClass* find_unit(const std::string& id);
    const UnitClass* find_unit(const std::string& id) const;

    double get_total_count() const;
    double get_total_initial_count() const;
    double get_total_casualties() const;
    double get_total_living_hp() const;
    double get_total_frontline_footprint() const;
    double get_total_raw_dps() const;

    double get_living_synapse_capacity() const;
    double get_required_synapse_capacity() const;

    double get_survival_percentage() const;
    bool is_defeated() const;
    void reset();
    void scale_all_units(double factor);

    // Factory presets
    static ArmyRoster create_imperial_defense(double scale = 1.0);
    static ArmyRoster create_tyranid_swarm(double scale = 1.0);
    static ArmyRoster create_ork_waaagh(double scale = 1.0);
    static ArmyRoster create_necron_dynasty(double scale = 1.0);
    static ArmyRoster create_chaos_incursion(double scale = 1.0);
    static ArmyRoster create_tau_empire(double scale = 1.0);
    static ArmyRoster create_aeldari_host(double scale = 1.0);
    static ArmyRoster create_custodes_vanguard(double scale = 1.0);
    static ArmyRoster create_genestealer_cult(double scale = 1.0);
    static ArmyRoster create_krieg_siege_army(double scale = 1.0);
    static ArmyRoster create_iron_warriors(double scale = 1.0);

    };

} // namespace omnisbelli

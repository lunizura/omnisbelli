#pragma once
#include <string>
#include <vector>
#include "engine/unit_class.hpp"

namespace omnisbelli {

class ArmyRoster {
public:
    std::string army_id;
    std::string display_name;
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
    bool has_living_units() const;
    void reset();

    // Standard formations
    static ArmyRoster create_imperial_defense(double scale = 1.0);
    static ArmyRoster create_tyranid_swarm(double scale = 1.0);
};

} // namespace omnisbelli

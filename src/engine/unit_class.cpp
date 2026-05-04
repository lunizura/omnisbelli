#include "engine/unit_class.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

UnitClass::UnitClass()
    : id(""), name("Unknown"), faction_id("neutral"),
      category(UnitCategory::INFANTRY),
      count(0.0), initial_count(0.0), hp_per_unit(10.0),
      armor(0.0), dps_per_unit(1.0), splash_factor(1.0),
      frontline_footprint(1.0), backline_ratio(0.0),
      target_priority(TargetPriority::PRIORITY_BALANCED),
      is_synapse(false), synapse_strength(0.0),
      ammo_type(AmmoType::AMMO_NONE), ammo_consumption_per_sec(0.0),
      casualties_suffered(0.0), total_damage_dealt(0.0) {}

UnitClass::UnitClass(
    const std::string& id,
    const std::string& name,
    const std::string& faction_id,
    UnitCategory category,
    double count,
    double hp_per_unit,
    double armor,
    double dps_per_unit,
    double splash_factor,
    double frontline_footprint,
    double backline_ratio,
    TargetPriority priority,
    bool is_synapse,
    double synapse_strength,
    AmmoType ammo_type,
    double ammo_consumption
)
    : id(id), name(name), faction_id(faction_id),
      category(category),
      count(count), initial_count(count), hp_per_unit(hp_per_unit),
      armor(std::clamp(armor, 0.0, 0.90)),
      dps_per_unit(dps_per_unit), splash_factor(std::max(1.0, splash_factor)),
      frontline_footprint(std::max(0.1, frontline_footprint)),
      backline_ratio(std::clamp(backline_ratio, 0.0, 0.95)),
      target_priority(priority),
      is_synapse(is_synapse), synapse_strength(synapse_strength),
      ammo_type(ammo_type), ammo_consumption_per_sec(ammo_consumption),
      casualties_suffered(0.0), total_damage_dealt(0.0) {}

double UnitClass::get_total_hp() const {
    return std::max(0.0, count) * hp_per_unit;
}

double UnitClass::get_effective_dps(double ammo_efficiency, double synergy_modifier) const {
    if (count <= 0.0) return 0.0;
    return count * dps_per_unit * ammo_efficiency * synergy_modifier;
}

double UnitClass::get_total_footprint() const {
    return std::max(0.0, count) * frontline_footprint;
}

double UnitClass::get_frontline_footprint_living() const {
    // Only frontline portion occupies immediate chokepoint frontage
    double front_ratio = 1.0 - backline_ratio;
    return std::max(0.0, count) * frontline_footprint * front_ratio;
}

double UnitClass::take_damage(double incoming_raw_damage, double armor_penetration) {
    if (count <= 0.0 || incoming_raw_damage <= 0.0) return 0.0;

    double effective_armor = std::clamp(armor - armor_penetration, 0.0, 0.90);
    double mitigated_damage = incoming_raw_damage * (1.0 - effective_armor);

    double units_killed = mitigated_damage / hp_per_unit;
    if (units_killed > count) {
        units_killed = count;
    }

    count -= units_killed;
    casualties_suffered += units_killed;
    if (count < 0.0001) {
        count = 0.0;
    }
    return units_killed;
}

double UnitClass::apply_direct_casualties(double count_to_kill) {
    if (count <= 0.0 || count_to_kill <= 0.0) return 0.0;
    double actual_killed = std::min(count, count_to_kill);
    count -= actual_killed;
    casualties_suffered += actual_killed;
    if (count < 0.0001) {
        count = 0.0;
    }
    return actual_killed;
}

bool UnitClass::is_alive() const {
    return count > 0.0001;
}

double UnitClass::get_survival_percentage() const {
    if (initial_count <= 0.0) return 0.0;
    return (count / initial_count) * 100.0;
}

void UnitClass::reset() {
    count = initial_count;
    casualties_suffered = 0.0;
    total_damage_dealt = 0.0;
}

} // namespace omnisbelli

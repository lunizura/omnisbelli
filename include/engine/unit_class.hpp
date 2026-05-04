#pragma once
#include <string>
#include <algorithm>

namespace omnisbelli {

enum class UnitCategory {
    INFANTRY,
    HEAVY_INFANTRY,
    ARMOR_VEHICLE,
    SUPER_HEAVY_TITAN,
    ARTILLERY_ORBITAL
};

enum class AmmoType {
    AMMO_NONE,
    AMMO_LASPACKS,
    AMMO_BOLTER_ROUNDS,
    AMMO_ARTILLERY_SHELLS,
    AMMO_PROMETHIUM,
    AMMO_PLASMA
};

enum class TargetPriority {
    PRIORITY_BALANCED,
    PRIORITY_SWARM,
    PRIORITY_HEAVY,
    PRIORITY_SYNAPSE
};

class UnitClass {
public:
    std::string id;
    std::string name;
    std::string faction_id;
    UnitCategory category;

    double count;
    double initial_count;
    double hp_per_unit;
    double armor;                       // Damage reduction [0.0, 0.90]
    double dps_per_unit;
    double splash_factor;               // Multiplier vs high-density swarms
    double frontline_footprint;         // Width in meters occupied per unit
    double backline_ratio;              // [0.0 = front, 0.9 = protected backline]
    TargetPriority target_priority;

    bool is_synapse;
    double synapse_strength;            // Subordinate swarm units sustained per entity
    AmmoType ammo_type;
    double ammo_consumption_per_sec;

    double casualties_suffered;
    double total_damage_dealt;

    UnitClass();
    UnitClass(
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
        bool is_synapse = false,
        double synapse_strength = 0.0,
        AmmoType ammo_type = AmmoType::AMMO_NONE,
        double ammo_consumption = 0.0
    );

    double get_total_hp() const;
    double get_effective_dps(double ammo_efficiency = 1.0, double synergy_modifier = 1.0) const;
    double get_total_footprint() const;
    double get_frontline_footprint_living() const;
    
    // Applies damage and returns actual units killed in this step
    double take_damage(double incoming_raw_damage, double armor_penetration = 0.0);
    
    // Direct casualty application (e.g. minefields, promethium fire, drop-pod decapitation)
    double apply_direct_casualties(double count_to_kill);

    bool is_alive() const;
    double get_survival_percentage() const;
    void reset();
};

} // namespace omnisbelli

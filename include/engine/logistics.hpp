#pragma once
#include "engine/unit_class.hpp"
#include <string>

namespace omnisbelli {

class LogisticsSystem {
public:
    // Reserves
    double laspack_mwh;
    double laspack_max_mwh;
    double laspack_recharge_per_sec;

    double bolter_rounds;
    double bolter_max_rounds;
    double bolter_resupply_per_sec;

    double artillery_shells;
    double artillery_max_shells;
    double artillery_resupply_per_sec;

    double promethium_liters;
    double promethium_max_liters;

    double titan_plasma_heat;           // [0.0, 100.0]
    double titan_plasma_cooling_per_sec;

    // Tracking
    double total_bolter_fired;
    double total_artillery_fired;
    double total_promethium_burned;

    LogisticsSystem();

    void update(double dt);
    double consume_ammo(AmmoType type, double amount);
    double get_efficiency(AmmoType type) const;

    bool can_afford_promethium(double liters) const;
    bool consume_promethium(double liters);

    void add_plasma_heat(double heat);
    bool is_plasma_overheated() const;

    double get_reserve_percentage(AmmoType type) const;
    void reset();
};

} // namespace omnisbelli

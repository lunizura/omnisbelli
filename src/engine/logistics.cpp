#include "engine/logistics.hpp"
#include <algorithm>

namespace omnisbelli {

LogisticsSystem::LogisticsSystem()
    : laspack_mwh(1000000.0), laspack_max_mwh(1000000.0), laspack_recharge_per_sec(5000.0),
      bolter_rounds(3500000.0), bolter_max_rounds(3500000.0), bolter_resupply_per_sec(200.0),
      artillery_shells(850000.0), artillery_max_shells(850000.0), artillery_resupply_per_sec(40.0),
      promethium_liters(120000.0), promethium_max_liters(120000.0),
      titan_plasma_heat(0.0), titan_plasma_cooling_per_sec(1.0),
      total_bolter_fired(0.0), total_artillery_fired(0.0), total_promethium_burned(0.0) {}

void LogisticsSystem::update(double dt) {
    if (dt <= 0.0) return;

    // Passive geothermal / nuclear recharge of laspacks
    laspack_mwh = std::min(laspack_max_mwh, laspack_mwh + laspack_recharge_per_sec * dt);

    // Underground Munitorium trickle
    bolter_rounds = std::min(bolter_max_rounds, bolter_rounds + bolter_resupply_per_sec * dt);
    artillery_shells = std::min(artillery_max_shells, artillery_shells + artillery_resupply_per_sec * dt);

    // Titan reactor coolant dissipation
    if (titan_plasma_heat > 0.0) {
        titan_plasma_heat = std::max(0.0, titan_plasma_heat - titan_plasma_cooling_per_sec * dt);
    }
}

double LogisticsSystem::consume_ammo(AmmoType type, double amount) {
    if (amount <= 0.0) return 1.0;

    switch (type) {
    case AmmoType::AMMO_LASPACKS:
        if (laspack_mwh >= amount) {
            laspack_mwh -= amount;
            return 1.0;
        } else {
            double ratio = (laspack_mwh > 0.0) ? (laspack_mwh / amount) : 0.0;
            laspack_mwh = 0.0;
            return std::max(0.2, ratio); // Emergency low-power las-fire
        }

    case AmmoType::AMMO_BOLTER_ROUNDS:
        if (bolter_rounds >= amount) {
            bolter_rounds -= amount;
            total_bolter_fired += amount;
            return 1.0;
        } else {
            double ratio = (bolter_rounds > 0.0) ? (bolter_rounds / amount) : 0.0;
            total_bolter_fired += bolter_rounds;
            bolter_rounds = 0.0;
            return std::max(0.25, ratio); // Troops switch to combat blades & backup sidearms
        }

    case AmmoType::AMMO_ARTILLERY_SHELLS:
        if (artillery_shells >= amount) {
            artillery_shells -= amount;
            total_artillery_fired += amount;
            return 1.0;
        } else {
            double ratio = (artillery_shells > 0.0) ? (artillery_shells / amount) : 0.0;
            total_artillery_fired += artillery_shells;
            artillery_shells = 0.0;
            return std::max(0.05, ratio); // Severe drop: lack of heavy munitions
        }

    case AmmoType::AMMO_PLASMA:
        if (titan_plasma_heat >= 100.0) {
            return 0.15; // Overheated emergency cooling shutdown
        }
        titan_plasma_heat = std::min(100.0, titan_plasma_heat + amount * 0.02);
        return 1.0;

    case AmmoType::AMMO_PROMETHIUM:
    case AmmoType::AMMO_NONE:
    default:
        return 1.0;
    }
}

double LogisticsSystem::get_efficiency(AmmoType type) const {
    switch (type) {
    case AmmoType::AMMO_ARTILLERY_SHELLS:
        return (artillery_shells > 0.0) ? 1.0 : 0.05;
    case AmmoType::AMMO_BOLTER_ROUNDS:
        return (bolter_rounds > 0.0) ? 1.0 : 0.35;
    case AmmoType::AMMO_LASPACKS:
        return (laspack_mwh > 0.0) ? 1.0 : 0.30;
    case AmmoType::AMMO_PLASMA:
        return (titan_plasma_heat < 90.0) ? 1.0 : 0.25;
    default:
        return 1.0;
    }
}

bool LogisticsSystem::can_afford_promethium(double liters) const {
    return promethium_liters >= liters;
}

bool LogisticsSystem::consume_promethium(double liters) {
    if (promethium_liters >= liters) {
        promethium_liters -= liters;
        total_promethium_burned += liters;
        return true;
    }
    return false;
}

void LogisticsSystem::add_plasma_heat(double heat) {
    titan_plasma_heat = std::min(100.0, titan_plasma_heat + heat);
}

bool LogisticsSystem::is_plasma_overheated() const {
    return titan_plasma_heat >= 85.0;
}

double LogisticsSystem::get_reserve_percentage(AmmoType type) const {
    switch (type) {
    case AmmoType::AMMO_LASPACKS:
        return (laspack_max_mwh > 0.0) ? (laspack_mwh / laspack_max_mwh) * 100.0 : 0.0;
    case AmmoType::AMMO_BOLTER_ROUNDS:
        return (bolter_max_rounds > 0.0) ? (bolter_rounds / bolter_max_rounds) * 100.0 : 0.0;
    case AmmoType::AMMO_ARTILLERY_SHELLS:
        return (artillery_max_shells > 0.0) ? (artillery_shells / artillery_max_shells) * 100.0 : 0.0;
    case AmmoType::AMMO_PROMETHIUM:
        return (promethium_max_liters > 0.0) ? (promethium_liters / promethium_max_liters) * 100.0 : 0.0;
    case AmmoType::AMMO_PLASMA:
        return std::max(0.0, 100.0 - titan_plasma_heat);
    default:
        return 100.0;
    }
}

void LogisticsSystem::reset() {
    laspack_mwh = laspack_max_mwh;
    bolter_rounds = bolter_max_rounds;
    artillery_shells = artillery_max_shells;
    promethium_liters = promethium_max_liters;
    titan_plasma_heat = 0.0;
    total_bolter_fired = 0.0;
    total_artillery_fired = 0.0;
    total_promethium_burned = 0.0;
}

} // namespace omnisbelli

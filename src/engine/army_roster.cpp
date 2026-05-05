#include "engine/army_roster.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

ArmyRoster::ArmyRoster()
    : army_id("unassigned"), display_name("Unassigned Force") {}

ArmyRoster::ArmyRoster(const std::string& army_id, const std::string& display_name)
    : army_id(army_id), display_name(display_name) {}

void ArmyRoster::add_unit(const UnitClass& unit) {
    units.push_back(unit);
}

UnitClass* ArmyRoster::find_unit(const std::string& id) {
    for (auto& u : units) {
        if (u.id == id) return &u;
    }
    return nullptr;
}

const UnitClass* ArmyRoster::find_unit(const std::string& id) const {
    for (const auto& u : units) {
        if (u.id == id) return &u;
    }
    return nullptr;
}

double ArmyRoster::get_total_count() const {
    double total = 0.0;
    for (const auto& u : units) {
        total += u.count;
    }
    return total;
}

double ArmyRoster::get_total_initial_count() const {
    double total = 0.0;
    for (const auto& u : units) {
        total += u.initial_count;
    }
    return total;
}

double ArmyRoster::get_total_casualties() const {
    double total = 0.0;
    for (const auto& u : units) {
        total += u.casualties_suffered;
    }
    return total;
}

double ArmyRoster::get_total_living_hp() const {
    double total = 0.0;
    for (const auto& u : units) {
        total += u.get_total_hp();
    }
    return total;
}

double ArmyRoster::get_total_frontline_footprint() const {
    double total = 0.0;
    for (const auto& u : units) {
        total += u.get_frontline_footprint_living();
    }
    return total;
}

double ArmyRoster::get_total_raw_dps() const {
    double total = 0.0;
    for (const auto& u : units) {
        total += u.get_effective_dps(1.0, 1.0);
    }
    return total;
}

double ArmyRoster::get_living_synapse_capacity() const {
    double capacity = 0.0;
    for (const auto& u : units) {
        if (u.is_synapse && u.is_alive()) {
            capacity += u.count * u.synapse_strength;
        }
    }
    return capacity;
}

double ArmyRoster::get_required_synapse_capacity() const {
    // Non-synapse swarm units require synapse oversight
    double swarm_count = 0.0;
    for (const auto& u : units) {
        if (!u.is_synapse) {
            swarm_count += u.count;
        }
    }
    return swarm_count;
}

double ArmyRoster::get_survival_percentage() const {
    double init = get_total_initial_count();
    if (init <= 0.0) return 0.0;
    return (get_total_count() / init) * 100.0;
}

bool ArmyRoster::is_defeated() const {
    return get_total_count() <= 1.0;
}

void ArmyRoster::reset() {
    for (auto& u : units) {
        u.reset();
    }
}

void ArmyRoster::scale_all_units(double factor) {
    if (factor <= 0.0) return;
    for (auto& u : units) {
        u.initial_count = std::round(u.initial_count * factor);
        u.count = u.initial_count;
    }
}

ArmyRoster ArmyRoster::create_imperial_defense(double scale) {
    ArmyRoster roster("imperium", "Imperium of Man - Forge World Defense");

    roster.add_unit(UnitClass(
        "cadian_infantry", "Cadian Shock Troops", "imperium",
        UnitCategory::INFANTRY,
        1000000.0 * scale, 15.0, 0.15, 3.2, 1.0, 1.0, 0.0,
        TargetPriority::PRIORITY_BALANCED,
        false, 0.0, AmmoType::AMMO_LASPACKS, 0.02
    ));

    roster.add_unit(UnitClass(
        "pdf_conscript", "PDF Militia Conscripts", "imperium",
        UnitCategory::INFANTRY,
        850000.0 * scale, 10.0, 0.05, 1.8, 1.0, 0.8, 0.0,
        TargetPriority::PRIORITY_SWARM,
        false, 0.0, AmmoType::AMMO_LASPACKS, 0.015
    ));

    roster.add_unit(UnitClass(
        "skitarii_vanguard", "Skitarii Vanguard Legions", "imperium",
        UnitCategory::HEAVY_INFANTRY,
        120000.0 * scale, 28.0, 0.35, 6.8, 1.2, 1.0, 0.10,
        TargetPriority::PRIORITY_BALANCED,
        false, 0.0, AmmoType::AMMO_LASPACKS, 0.03
    ));

    roster.add_unit(UnitClass(
        "leman_russ", "Leman Russ Battle Tanks", "imperium",
        UnitCategory::ARMOR_VEHICLE,
        2500.0 * scale, 3500.0, 0.75, 290.0, 4.0, 6.0, 0.30,
        TargetPriority::PRIORITY_HEAVY,
        false, 0.0, AmmoType::AMMO_BOLTER_ROUNDS, 1.2
    ));

    roster.add_unit(UnitClass(
        "basilisk_artillery", "Basilisk Artillery Batteries", "imperium",
        UnitCategory::ARTILLERY_ORBITAL,
        4500.0 * scale, 1800.0, 0.40, 480.0, 10.0, 8.0, 0.85,
        TargetPriority::PRIORITY_SWARM,
        false, 0.0, AmmoType::AMMO_ARTILLERY_SHELLS, 0.8
    ));

    roster.add_unit(UnitClass(
        "astartes_spearhead", "Space Marine Spearhead", "imperium",
        UnitCategory::HEAVY_INFANTRY,
        1000.0 * scale, 150.0, 0.70, 85.0, 1.5, 1.5, 0.0,
        TargetPriority::PRIORITY_SYNAPSE,
        false, 0.0, AmmoType::AMMO_BOLTER_ROUNDS, 0.4
    ));

    roster.add_unit(UnitClass(
        "warhound_titan", "Warhound Titan Maniple", "imperium",
        UnitCategory::SUPER_HEAVY_TITAN,
        20.0 * scale, 55000.0, 0.85, 6800.0, 15.0, 35.0, 0.50,
        TargetPriority::PRIORITY_HEAVY,
        false, 0.0, AmmoType::AMMO_PLASMA, 2.5
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_tyranid_swarm(double scale) {
    ArmyRoster roster("tyranids", "Hive Fleet Leviathan Swarm");

    roster.add_unit(UnitClass(
        "hormagaunt", "Hormagaunt Swarmers", "tyranids",
        UnitCategory::INFANTRY,
        65000000.0 * scale, 8.0, 0.05, 1.2, 1.0, 0.5, 0.0,
        TargetPriority::PRIORITY_BALANCED,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "termagant", "Termagant Bio-Gunners", "tyranids",
        UnitCategory::INFANTRY,
        34000000.0 * scale, 8.0, 0.05, 1.4, 1.0, 0.6, 0.10,
        TargetPriority::PRIORITY_BALANCED,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "tyranid_warrior", "Tyranid Warriors (Synapse)", "tyranids",
        UnitCategory::HEAVY_INFANTRY,
        850000.0 * scale, 160.0, 0.45, 22.0, 1.8, 2.0, 0.20,
        TargetPriority::PRIORITY_BALANCED,
        true, 120.0 // Sustains 120 lesser bio-forms each
    ));

    roster.add_unit(UnitClass(
        "carnifex", "Carnifex Siege Monsters", "tyranids",
        UnitCategory::ARMOR_VEHICLE,
        140000.0 * scale, 4500.0, 0.65, 320.0, 6.0, 8.0, 0.10,
        TargetPriority::PRIORITY_HEAVY,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "zoanthrope", "Zoanthrope Warp-Synapse", "tyranids",
        UnitCategory::HEAVY_INFANTRY,
        9000.0 * scale, 850.0, 0.60, 180.0, 4.0, 3.0, 0.60,
        TargetPriority::PRIORITY_BALANCED,
        true, 2500.0 // Sustains 2,500 lesser bio-forms each
    ));

    roster.add_unit(UnitClass(
        "hierophant_biotitan", "Hierophant Bio-Titans", "tyranids",
        UnitCategory::SUPER_HEAVY_TITAN,
        1000.0 * scale, 95000.0, 0.80, 8500.0, 18.0, 40.0, 0.40,
        TargetPriority::PRIORITY_HEAVY,
        true, 30000.0
    ));

    roster.add_unit(UnitClass(
        "swarmlord_apex", "Swarmlord Apex Nodes", "tyranids",
        UnitCategory::SUPER_HEAVY_TITAN,
        20.0 * scale, 15000.0, 0.75, 1800.0, 3.0, 4.0, 0.30,
        TargetPriority::PRIORITY_SYNAPSE,
        true, 600000.0 // Supreme synapse nexus
    ));

    return roster;
}


} // namespace omnisbelli

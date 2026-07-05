#include "engine/army_roster.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

ArmyRoster::ArmyRoster()
    : army_id("unassigned"), display_name("Unassigned Force"),
      faction_id("unassigned"), name("Unassigned Force") {}

ArmyRoster::ArmyRoster(const std::string& army_id, const std::string& display_name)
    : army_id(army_id), display_name(display_name),
      faction_id(army_id), name(display_name) {}

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

ArmyRoster ArmyRoster::create_ork_waaagh(double scale) {
    ArmyRoster roster("orks", "WAAAGH! Ghazghkull Planetary Invasion");

    roster.add_unit(UnitClass(
        "ork_boyz", "Ork Boyz Mob", "orks",
        UnitCategory::INFANTRY,
        45000000.0 * scale, 12.0, 0.10, 2.0, 1.0, 0.8, 0.0,
        TargetPriority::PRIORITY_BALANCED,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "ork_nobz", "Ork Nobz Brawlers", "orks",
        UnitCategory::HEAVY_INFANTRY,
        1200000.0 * scale, 90.0, 0.40, 18.0, 1.5, 1.8, 0.1,
        TargetPriority::PRIORITY_BALANCED,
        true, 50.0 // WAAAGH! focal leader
    ));

    roster.add_unit(UnitClass(
        "battlewagon", "Kustom Battlewagons", "orks",
        UnitCategory::ARMOR_VEHICLE,
        80000.0 * scale, 3000.0, 0.60, 240.0, 5.0, 7.0, 0.2,
        TargetPriority::PRIORITY_HEAVY,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "gargant", "Great Gargant God-Machines", "orks",
        UnitCategory::SUPER_HEAVY_TITAN,
        500.0 * scale, 110000.0, 0.82, 9500.0, 20.0, 45.0, 0.4,
        TargetPriority::PRIORITY_HEAVY,
        true, 100000.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_necron_dynasty(double scale) {
    ArmyRoster roster("necrons", "Sautekh Necron Dynasty Legion");

    roster.add_unit(UnitClass(
        "necron_warrior", "Necron Gauss Warriors", "necrons",
        UnitCategory::INFANTRY,
        15000000.0 * scale, 24.0, 0.30, 4.5, 1.0, 1.0, 0.0,
        TargetPriority::PRIORITY_BALANCED,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "immortals", "Tesla Immortals", "necrons",
        UnitCategory::HEAVY_INFANTRY,
        2500000.0 * scale, 65.0, 0.50, 14.0, 2.0, 1.4, 0.1,
        TargetPriority::PRIORITY_BALANCED,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "monolith", "Necron Monolith Siege Engines", "necrons",
        UnitCategory::SUPER_HEAVY_TITAN,
        15000.0 * scale, 45000.0, 0.85, 4200.0, 12.0, 20.0, 0.3,
        TargetPriority::PRIORITY_HEAVY,
        true, 50000.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_chaos_incursion(double scale) {
    ArmyRoster roster("chaos", "Black Legion Warp Incursion");

    roster.add_unit(UnitClass(
        "cultists", "Lost and the Damned Cultists", "chaos",
        UnitCategory::INFANTRY,
        40000000.0 * scale, 9.0, 0.05, 1.5, 1.0, 0.7, 0.0,
        TargetPriority::PRIORITY_SWARM,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "chaos_space_marines", "Heretic Legionnaires", "chaos",
        UnitCategory::HEAVY_INFANTRY,
        2000000.0 * scale, 160.0, 0.68, 75.0, 1.5, 1.5, 0.1,
        TargetPriority::PRIORITY_BALANCED,
        true, 100.0
    ));

    roster.add_unit(UnitClass(
        "helbrute", "Helbrute Daemon Engines", "chaos",
        UnitCategory::ARMOR_VEHICLE,
        150000.0 * scale, 3800.0, 0.65, 310.0, 5.0, 6.0, 0.2,
        TargetPriority::PRIORITY_HEAVY,
        false, 0.0
    ));

    roster.add_unit(UnitClass(
        "greater_daemon", "Greater Daemons of the Warp", "chaos",
        UnitCategory::SUPER_HEAVY_TITAN,
        3000.0 * scale, 85000.0, 0.82, 7800.0, 16.0, 25.0, 0.4,
        TargetPriority::PRIORITY_HEAVY,
        true, 50000.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_tau_empire(double scale) {
    ArmyRoster roster("tau", "T'au Empire Hunter Cadre");

    roster.add_unit(UnitClass(
        "fire_warriors", "Strike Team Fire Warriors", "tau",
        UnitCategory::INFANTRY,
        1500000.0 * scale, 12.0, 0.35, 8.5, 1.2, 0.9, 0.0,
        TargetPriority::PRIORITY_SWARM, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "crisis_battlesuits", "XV8 Crisis Battlesuits", "tau",
        UnitCategory::HEAVY_INFANTRY,
        120000.0 * scale, 140.0, 0.60, 95.0, 3.5, 2.5, 0.15,
        TargetPriority::PRIORITY_BALANCED, true, 50.0
    ));

    roster.add_unit(UnitClass(
        "hammerhead_gunship", "Hammerhead Railgun Tanks", "tau",
        UnitCategory::ARMOR_VEHICLE,
        18000.0 * scale, 4200.0, 0.70, 850.0, 8.0, 8.0, 0.25,
        TargetPriority::PRIORITY_HEAVY, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "riptide_battlesuit", "XV104 Riptide Battlesuits", "tau",
        UnitCategory::SUPER_HEAVY_TITAN,
        2500.0 * scale, 38000.0, 0.80, 3400.0, 10.0, 15.0, 0.35,
        TargetPriority::PRIORITY_HEAVY, true, 25000.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_aeldari_host(double scale) {
    ArmyRoster roster("aeldari", "Asuryani Craftworld Host");

    roster.add_unit(UnitClass(
        "guardians", "Guardian Defenders", "aeldari",
        UnitCategory::INFANTRY,
        1000000.0 * scale, 11.0, 0.28, 9.0, 1.5, 0.8, 0.0,
        TargetPriority::PRIORITY_SWARM, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "dire_avengers", "Dire Avenger Aspect Warriors", "aeldari",
        UnitCategory::HEAVY_INFANTRY,
        250000.0 * scale, 24.0, 0.45, 22.0, 2.0, 1.0, 0.05,
        TargetPriority::PRIORITY_BALANCED, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "wraithguard", "Wraithguard Construct Host", "aeldari",
        UnitCategory::ARMOR_VEHICLE,
        45000.0 * scale, 520.0, 0.75, 280.0, 6.0, 2.0, 0.15,
        TargetPriority::PRIORITY_HEAVY, true, 100.0
    ));

    roster.add_unit(UnitClass(
        "avatar_khaine", "Avatar of the Bloody-Handed God", "aeldari",
        UnitCategory::SUPER_HEAVY_TITAN,
        10.0 * scale, 65000.0, 0.88, 9200.0, 18.0, 8.0, 0.5,
        TargetPriority::PRIORITY_HEAVY, true, 100000.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_custodes_vanguard(double scale) {
    ArmyRoster roster("custodes", "Adeptus Custodes Shield Host");

    roster.add_unit(UnitClass(
        "custodian_guard", "Custodian Guard Shield Companions", "custodes",
        UnitCategory::HEAVY_INFANTRY,
        10000.0 * scale, 450.0, 0.85, 320.0, 4.0, 1.5, 0.25,
        TargetPriority::PRIORITY_BALANCED, true, 500.0
    ));

    roster.add_unit(UnitClass(
        "allarus_terminator", "Allarus Custodians", "custodes",
        UnitCategory::HEAVY_INFANTRY,
        3500.0 * scale, 750.0, 0.88, 480.0, 5.0, 2.0, 0.30,
        TargetPriority::PRIORITY_SYNAPSE, true, 1000.0
    ));

    roster.add_unit(UnitClass(
        "telemon_dreadnought", "Telemon Heavy Dreadnoughts", "custodes",
        UnitCategory::SUPER_HEAVY_TITAN,
        300.0 * scale, 48000.0, 0.90, 4500.0, 12.0, 12.0, 0.40,
        TargetPriority::PRIORITY_HEAVY, true, 20000.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_genestealer_cult(double scale) {
    ArmyRoster roster("gsc", "Four-Armed Emperor Insurrection");

    roster.add_unit(UnitClass(
        "neophytes", "Neophyte Hybrid Insurgents", "gsc",
        UnitCategory::INFANTRY,
        15000000.0 * scale, 8.0, 0.10, 1.8, 1.0, 0.7, 0.0,
        TargetPriority::PRIORITY_SWARM, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "aberrants", "Aberrant Shock Mutants", "gsc",
        UnitCategory::HEAVY_INFANTRY,
        800000.0 * scale, 85.0, 0.40, 45.0, 2.0, 1.6, 0.05,
        TargetPriority::PRIORITY_BALANCED, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "rockgrinder", "Goliath Rockgrinder Drills", "gsc",
        UnitCategory::ARMOR_VEHICLE,
        45000.0 * scale, 2800.0, 0.55, 240.0, 4.0, 6.0, 0.2,
        TargetPriority::PRIORITY_HEAVY, false, 0.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_krieg_siege_army(double scale) {
    ArmyRoster roster("krieg", "Death Korps of Krieg 88th Siege Army");

    roster.add_unit(UnitClass(
        "krieg_infantry", "Krieg Trench Guardsmen (Lucius No. 98)", "krieg",
        UnitCategory::INFANTRY,
        3500000.0 * scale, 12.0, 0.25, 2.8, 1.0, 0.8, 0.0,
        TargetPriority::PRIORITY_SWARM, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "death_riders", "Death Rider Cavalry", "krieg",
        UnitCategory::HEAVY_INFANTRY,
        120000.0 * scale, 45.0, 0.45, 38.0, 2.5, 1.8, 0.1,
        TargetPriority::PRIORITY_BALANCED, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "krieg_earthshaker", "Earthshaker Carriage Batteries", "krieg",
        UnitCategory::ARTILLERY_ORBITAL,
        12000.0 * scale, 1800.0, 0.40, 850.0, 8.0, 8.0, 0.3,
        TargetPriority::PRIORITY_SWARM, false, 0.0
    ));

    return roster;
}

ArmyRoster ArmyRoster::create_iron_warriors(double scale) {
    ArmyRoster roster("iron_warriors", "Iron Warriors IV Legion Grand Battalion");

    roster.add_unit(UnitClass(
        "iron_legionnaires", "Iron Warriors Legion Breachers", "iron_warriors",
        UnitCategory::HEAVY_INFANTRY,
        450000.0 * scale, 180.0, 0.72, 85.0, 2.0, 1.5, 0.1,
        TargetPriority::PRIORITY_BALANCED, true, 200.0
    ));

    roster.add_unit(UnitClass(
        "havoc_squads", "Havoc Heavy Support Batteries", "iron_warriors",
        UnitCategory::HEAVY_INFANTRY,
        80000.0 * scale, 160.0, 0.68, 190.0, 6.0, 2.0, 0.2,
        TargetPriority::PRIORITY_HEAVY, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "defiler_engine", "Defiler Siege Daemon Engines", "iron_warriors",
        UnitCategory::ARMOR_VEHICLE,
        25000.0 * scale, 6500.0, 0.70, 720.0, 7.0, 8.0, 0.3,
        TargetPriority::PRIORITY_HEAVY, false, 0.0
    ));

    return roster;
}


ArmyRoster ArmyRoster::create_death_guard(double scale) {
    ArmyRoster roster("death_guard", "Death Guard XIV Legion Plague Vectorium");

    roster.add_unit(UnitClass(
        "poxwalkers", "Poxwalker Plague Thralls", "death_guard",
        UnitCategory::INFANTRY,
        12000000.0 * scale, 14.0, 0.05, 1.4, 1.0, 0.6, 0.0,
        TargetPriority::PRIORITY_SWARM, false, 0.0
    ));

    roster.add_unit(UnitClass(
        "plague_marines", "Death Guard Plague Marine Vectors", "death_guard",
        UnitCategory::HEAVY_INFANTRY,
        1500000.0 * scale, 220.0, 0.72, 82.0, 2.0, 1.6, 0.1,
        TargetPriority::PRIORITY_BALANCED, true, 300.0
    ));

    roster.add_unit(UnitClass(
        "deathshroud", "Deathshroud Terminator Bodyguard", "death_guard",
        UnitCategory::HEAVY_INFANTRY,
        60000.0 * scale, 580.0, 0.85, 310.0, 3.5, 2.2, 0.05,
        TargetPriority::PRIORITY_SYNAPSE, true, 2000.0
    ));

        return roster;
}

} // namespace omnisbelli

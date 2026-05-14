#include "engine/command_orders.hpp"
#include <algorithm>

namespace omnisbelli {

CommandOrders::CommandOrders()
    : exterminatus_triggered(false),
      last_action_message("TACTICAL COGITATOR ONLINE. AWAITING ORDERS.") {

    doctrines.push_back({
        DoctrineType::IGNITE_PROMETHIUM_MOAT,
        "F1",
        "IGNITE PROMETHIUM MOAT",
        "Burns 2.5M gaunts, costs 15,000L fuel, repairs 10% breach.",
        30.0, 0.0, true
    });

    doctrines.push_back({
        DoctrineType::TITAN_PLASMA_VOLLEY,
        "F2",
        "TITAN PLASMA VOLLEY",
        "Overcharges Titan plasma reactors. Vaporizes heavy bio-titans.",
        45.0, 0.0, true
    });

    doctrines.push_back({
        DoctrineType::ASTARTES_DROP_POD,
        "F3",
        "ASTARTES DROP-POD SPEARHEAD",
        "Decapitation strike targeting Synapse nodes (Warriors/Zoanthropes).",
        60.0, 0.0, true
    });

    doctrines.push_back({
        DoctrineType::TACTICAL_FALLBACK,
        "F4",
        "ORDER TACTICAL FALLBACK",
        "Orderly withdrawal to next fortified sector. Saves surviving troops.",
        15.0, 0.0, true
    });

    doctrines.push_back({
        DoctrineType::ORBITAL_LANCE_STRIKE,
        "F5",
        "ORBITAL LANCE BATTERY",
        "Imperial Navy orbital lance fire. 8M swarm kills, collateral danger.",
        90.0, 0.0, true
    });

    doctrines.push_back({
        DoctrineType::EXTERMINATUS_PROTOCOL,
        "F8",
        "EXTERMINATUS PROTOCOL",
        "Cleanses the planet with cyclonic torpedoes. Denies biomass.",
        999.0, 0.0, false
    });
}

void CommandOrders::update(double dt, const SectorGraph& sector_graph) {
    if (dt <= 0.0) return;

    for (auto& doc : doctrines) {
        if (doc.cooldown_timer > 0.0) {
            doc.cooldown_timer = std::max(0.0, doc.cooldown_timer - dt);
        }

        if (false && doc.type == DoctrineType::EXTERMINATUS_PROTOCOL) {
            // Unlockable once Bastion (sector index >= 3) has fallen or active sector is Citadel
            doc.is_available = (sector_graph.active_sector_index >= 3) && !exterminatus_triggered;
        } else {
            doc.is_available = (doc.cooldown_timer <= 0.0);
        }
    }
}

bool CommandOrders::trigger_doctrine(
    DoctrineType type,
    ArmyRoster& defenders,
    ArmyRoster& attackers,
    SectorGraph& sector_graph,
    LogisticsSystem& logistics,
    SynapseNetwork& synapse
) {
    auto* doc = const_cast<DoctrineInfo*>(get_doctrine(type));
    if (!doc || !doc->is_available || doc->cooldown_timer > 0.0) return false;

    switch (type) {
    case DoctrineType::IGNITE_PROMETHIUM_MOAT: {
        if (!logistics.can_afford_promethium(15000.0)) {
            last_action_message = "[ERROR] INSUFFICIENT PROMETHIUM TO IGNITE MOAT!";
            return false;
        }
        logistics.consume_promethium(15000.0);
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;

        // Incinerate 2,500,000 swarm gaunts
        double to_kill = 2500000.0;
        double actual_killed = 0.0;
        for (auto& u : attackers.units) {
            if (u.category == UnitCategory::INFANTRY && u.is_alive()) {
                double killed = u.apply_direct_casualties(to_kill);
                actual_killed += killed;
                to_kill -= killed;
                if (to_kill <= 0.0) break;
            }
        }
        // Delay breach by restoring structural integrity
        auto& sec = sector_graph.get_active_sector();
        double restored = sec.max_integrity * 0.12;
        sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + restored);
        sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
        last_action_message = "[ORDER EXECUTED] PROMETHIUM MOAT IGNITED: " +
            std::to_string((int)actual_killed) + " SWARM KILLS. BREACH REDUCED.";
        return true;
    }

    case DoctrineType::TITAN_PLASMA_VOLLEY: {
        if (logistics.is_plasma_overheated()) {
            last_action_message = "[ERROR] TITAN REACTORS OVERHEATED! CANNOT DISCHARGE!";
            return false;
        }
        logistics.add_plasma_heat(35.0);
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;

        // Vaporize heavy bio-titans & carnifexes (with modular faction fallback)
        double carnifex_kills = 0.0;
        double biotitan_kills = 0.0;
        auto* carn = attackers.find_unit("carnifex");
        if (carn) carnifex_kills = carn->apply_direct_casualties(3500.0);
        auto* bio = attackers.find_unit("hierophant_biotitan");
        if (bio) biotitan_kills = bio->apply_direct_casualties(120.0);

        if (carnifex_kills == 0.0 && biotitan_kills == 0.0) {
            for (auto& u : attackers.units) {
                if (u.is_alive()) {
                    if (u.category == UnitCategory::SUPER_HEAVY_TITAN) {
                        biotitan_kills += u.apply_direct_casualties(std::min(u.count, 20.0));
                    } else if (u.category == UnitCategory::ARMOR_VEHICLE) {
                        carnifex_kills += u.apply_direct_casualties(std::min(u.count, 2000.0));
                    }
                }
            }
        }

        last_action_message = "[ORDER EXECUTED] TITAN PLASMA VOLLEY: VAPORIZED " +
            std::to_string((int)carnifex_kills) + " HEAVY ARMOR & " +
            std::to_string((int)biotitan_kills) + " TITANIC ASSETS.";
        return true;
    }

    case DoctrineType::ASTARTES_DROP_POD: {
        if (logistics.bolter_rounds < 25000.0) {
            last_action_message = "[ERROR] INSUFFICIENT BOLTER AMMUNITION FOR DROP PODS!";
            return false;
        }
        logistics.consume_ammo(AmmoType::AMMO_BOLTER_ROUNDS, 25000.0);
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;

        // Decapitation of Synapse beasts / High-Value Commanders
        double warriors_killed = 0.0;
        double zoans_killed = 0.0;
        auto* war = attackers.find_unit("tyranid_warrior");
        if (war) warriors_killed = war->apply_direct_casualties(15000.0);
        auto* zoan = attackers.find_unit("zoanthrope");
        if (zoan) zoans_killed = zoan->apply_direct_casualties(1800.0);

        if (warriors_killed == 0.0 && zoans_killed == 0.0) {
            for (auto& u : attackers.units) {
                if (u.is_alive() && (u.is_synapse || u.target_priority == TargetPriority::PRIORITY_SYNAPSE || u.category == UnitCategory::SUPER_HEAVY_TITAN)) {
                    warriors_killed += u.apply_direct_casualties(std::min(u.count, 5000.0));
                }
            }
            if (warriors_killed == 0.0) {
                for (auto& u : attackers.units) {
                    if (u.is_alive() && u.category == UnitCategory::HEAVY_INFANTRY) {
                        warriors_killed += u.apply_direct_casualties(std::min(u.count, 10000.0));
                        break;
                    }
                }
            }
        }

        synapse.update(attackers, 0.1);
        last_action_message = "[ORDER EXECUTED] SPACE MARINE SPEARHEAD: ELIMINATED " +
            std::to_string((int)(warriors_killed + zoans_killed)) + " PRIORITY COMMANDERS. ENEMY SHOCKED!";
        return true;
    }

    case DoctrineType::TACTICAL_FALLBACK: {
        if (sector_graph.active_sector_index >= sector_graph.sectors.size() - 1) {
            last_action_message = "[ERROR] CITADEL IS FINAL SECTOR. NO FURTHER RETREAT POSSIBLE!";
            return false;
        }
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;
        sector_graph.fallback_to_next_sector();
        last_action_message = "[ORDER EXECUTED] TACTICAL FALLBACK INITIATED TO " +
            sector_graph.get_active_sector().name + ". DEFENSIVE LINE RE-FORMED.";
        return true;
    }

    case DoctrineType::ORBITAL_LANCE_STRIKE: {
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;

        // Incinerate 8,000,000 swarm units
        double to_kill = 8000000.0;
        double actual_swarm = 0.0;
        for (auto& u : attackers.units) {
            if (u.is_alive()) {
                double killed = u.apply_direct_casualties(to_kill);
                actual_swarm += killed;
                to_kill -= killed;
                if (to_kill <= 0.0) break;
            }
        }
        // Collateral damage to friendly frontline
        double collateral = 0.0;
        auto* cadians = defenders.find_unit("cadian_infantry");
        if (cadians) collateral += cadians->apply_direct_casualties(35000.0);
        auto* conscripts = defenders.find_unit("pdf_conscript");
        if (conscripts) collateral += conscripts->apply_direct_casualties(45000.0);

        last_action_message = "[ORDER EXECUTED] ORBITAL LANCE STRIKE: " +
            std::to_string((int)actual_swarm) + " SWARM KILLS. " +
            std::to_string((int)collateral) + " FRIENDLY CASUALTIES.";
        return true;
    }

    case DoctrineType::EXTERMINATUS_PROTOCOL: {
        exterminatus_triggered = true;
        doc->is_available = false;

        // Cleanse both armies and all sectors
        for (auto& u : attackers.units) {
            u.apply_direct_casualties(u.count);
        }
        for (auto& u : defenders.units) {
            u.apply_direct_casualties(u.count);
        }
        for (auto& s : sector_graph.sectors) {
            s.status = SectorStatus::FALLEN;
            s.breach_percentage = 100.0;
            s.current_integrity = 0.0;
        }
        last_action_message = "[CRITICAL] EXTERMINATUS AUTHORIZED. CYCLONIC TORPEDOES DETONATED. PLANET PURGED.";
        return true;
    }

    default:
        return false;
    }
}

const DoctrineInfo* CommandOrders::get_doctrine(DoctrineType type) const {
    for (const auto& doc : doctrines) {
        if (doc.type == type) return &doc;
    }
    return nullptr;
}

void CommandOrders::reset() {
    exterminatus_triggered = false;
    last_action_message = "TACTICAL COGITATOR ONLINE. AWAITING ORDERS.";
    for (auto& doc : doctrines) {
        doc.cooldown_timer = 0.0;
        doc.is_available = (doc.type != DoctrineType::EXTERMINATUS_PROTOCOL);
    }
}

} // namespace omnisbelli

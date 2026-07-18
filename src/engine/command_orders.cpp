#include "engine/command_orders.hpp"
#include "engine/faction_rule_module.hpp"
#include "engine/ork_waaagh_mechanic.hpp"
#include "engine/necron_dynasty_mechanic.hpp"
#include "engine/tau_empire_mechanic.hpp"
#include "engine/custodes_mechanic.hpp"
#include "engine/death_guard_mechanic.hpp"
#include "engine/sororitas_mechanic.hpp"
#include "engine/mechanicus_mechanic.hpp"
#include "engine/world_eaters_mechanic.hpp"
#include <algorithm>

namespace omnisbelli {

CommandOrders::CommandOrders()
    : current_faction_id("imperium"),
      exterminatus_triggered(false),
      last_action_message("TACTICAL COGITATOR ONLINE. AWAITING ORDERS.") {
    configure_for_faction("imperium");
}

void CommandOrders::configure_for_faction(const std::string& faction_id) {
    current_faction_id = faction_id;
    doctrines.clear();

    if (faction_id == "orks") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "DA BIG WAAAGH! ROAR",
            "Bellows psychic roar: sets WAAAGH! frenzy, +40% melee DPS, repels vanguard.",
            30.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "ROKKS FROM DA SKY",
            "Hollowed asteroid crash: crushes heavy armor and titan-class bio-forms.",
            45.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "GREEN TIDE MOB CALL",
            "Rallies 2,000,000 Ork Boy reinforcements from scrap barricades.",
            50.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "TACTICAL BRAWL REPOSITION",
            "Mobs fall back laughing to next sector to set up ambush scrap barricades.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "DAKKA BURST STORM",
            "All shootas open up in deafening volley: 7M kills, collateral scrap.",
            80.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "GARGANT BELLY GUN PURGE",
            "God-machine discharges apocalyptic belly gun, scouring the sector.",
            999.0, 0.0, false
        });
    } else if (faction_id == "necrons") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "REANIMATION OVERCHARGE",
            "Dynastic surge: resurrects 1.5M fallen Necron warriors instantly.",
            30.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "DOOMSDAY ARKANITE BEAM",
            "Focused particle accelerator: obliterates heavy bio-titans & armor.",
            45.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "ETERNITY GATE INGRESS",
            "Monolith dimensional gate summons 250,000 Tesla Immortals.",
            60.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "DIMENSIONAL DISPERSAL",
            "Phases legion out of phase space to rematerialize at next redoubt.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "SOLAR PULSE GAUSS FLARE",
            "Tomb flare flays 8M enemy bio-forms with searing emerald radiation.",
            90.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "CELESTIAL ORRERY PURGE",
            "Manipulates cosmic strings to collapse the local crust into ash.",
            999.0, 0.0, false
        });
    } else if (faction_id == "tau" || faction_id == "tau_empire") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "MARKERLIGHT SATURATION",
            "Drones saturate targets: +5 Markerlight tokens, maximizing AP.",
            25.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "SEEKER MISSILE VOLLEY",
            "Sky Ray gunships fire smart missiles at heavy armored monsters.",
            40.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "CRISIS BATTLESUIT DROP",
            "Manta transports insert 20,000 XV8 Battlesuits with fusion blasters.",
            55.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "KAUYON FEIGNED RETREAT",
            "Patient Hunter lures invaders into prepared crossfire killzones.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "TIGER SHARK RAIL BARRAGE",
            "High-altitude aircraft fire heavy sub-munitions: 7.5M kills.",
            85.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "ION SPHERE CORE OVERLOAD",
            "Detonates planetary shield core to vaporize the entire warzone.",
            999.0, 0.0, false
        });
    } else if (faction_id == "death_guard") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "MIASMA OF PESTILENCE",
            "Corrosive cloud dissolves attackers (-25% breach, 2M kills).",
            30.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "PLAGUEBURST PHOSPHEX BARRAGE",
            "Siege mortars drown enemy heavy bio-constructs in caustic bile.",
            45.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "DEATHSHROUD TELEPORT SPEARHEAD",
            "Elite scythe terminators teleport into commander synapses.",
            60.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "SLIME-TRAIL WITHDRAWAL",
            "Plague marines withdraw slowly through noxious slime.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "ROTTING CORPSE CANNONADE",
            "Heavy catapults launch viral pestilence: 7M kills, spreads rot.",
            90.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "LIFE-EATER VIRAL INFECTION",
            "Nurgle's ultimate plague sweeps the planet, dissolving all flesh.",
            999.0, 0.0, false
        });
    } else if (faction_id == "world_eaters" || faction_id == "khorne") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "BLOOD FOR THE BLOOD GOD!",
            "Berserkers frenzy: grants +25 Blood Tithe points & +50% melee DPS.",
            25.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "LORD OF SKULLS CANNONADE",
            "Daemon engine gorestorm cannons shred heavy bio-armor.",
            45.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "RED BUTCHER ASSAULT DROP",
            "Assault claws ram into enemy lines delivering frenzied Terminators.",
            50.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "BLOODY TACTICAL FEINT",
            "Berserkers fall back to lure enemy into tighter chokepoint slaughter.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "BRASS METEOR STRIKE",
            "Khorne's brass meteors plunge from the warp: 8.5M kills.",
            80.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "SKULL THRONE APOTHEOSIS",
            "Planet is sacrificed to the Skull Throne in apocalyptic blood storm.",
            999.0, 0.0, false
        });
    } else if (faction_id == "sororitas" || faction_id == "adepta_sororitas") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "ACT OF FAITH: DIVINE AEGIS",
            "Emperor's blessing: +20 Miracle points, activates 4++ aegis shield.",
            30.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "EXORCIST PIPE ORGAN VOLLEY",
            "Sanctified missile barrage pulverizes enemy heavy columns.",
            45.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "SERAPHIM CELESTIAL DESCENT",
            "Jump-pack warriors descend with inferno pistols to purge synaptics.",
            55.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "SACRED MARTYRS RETREAT",
            "Orderly withdrawal under chanted litanies and sacred banners.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "RETRIBUTOR HOLY FIRE STORM",
            "Multi-melta and heavy flamers cleanse 8M heretics/xenos.",
            90.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "LIVING SAINT DIVINE WRATH",
            "Saint Celestine summons pillars of divine light purging all life.",
            999.0, 0.0, false
        });
    } else if (faction_id == "mechanicus" || faction_id == "adeptus_mechanicus") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "BINHARIC CANTICLE OVERRIDE",
            "Chants Shroudpsalm: +30% armor save and +25% galvanic DPS.",
            30.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "ORDINATUS MACRO-VOLCANO BEAM",
            "Titan-slaying macro-weapon annihilates priority super-heavies.",
            45.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "CYBERNETICA DATA-PULSE",
            "Awakens 10,000 Kastelan Robots and Skitarii cohort reinforcements.",
            55.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "CALCULATED LOGICAL RETREAT",
            "Algorithmic withdrawal optimizing firing corridors and kill rates.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "RAD-BOMB CARPET SATURATION",
            "Drops cobalt radiation warheads: 8M kills and radioactive ash.",
            90.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "OMNISSIAH DATA-PURGE PROTOCOL",
            "Thermal runaway detonation cleanses forge world cogitators.",
            999.0, 0.0, false
        });
    } else if (faction_id == "custodes" || faction_id == "adeptus_custodes") {
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "MARTIAL KA'TAH EMBOLUS",
            "Locks auramite shields into unbreakable wall: restores 15% breach.",
            30.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TITAN_PLASMA_VOLLEY,
            "F2",
            "TELEMON STORM CANNON BURST",
            "Heavy arachnus storm lasers blast heavy bio-titans & armor.",
            45.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ASTARTES_DROP_POD,
            "F3",
            "ALLARUS SHADOWKEEP INGRESS",
            "Golden terminators teleport directly onto enemy commanders.",
            60.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::TACTICAL_FALLBACK,
            "F4",
            "STRATEGIC RETROGRADE MOVEMENT",
            "Flawless tactical repositioning preserving all shield-brothers.",
            15.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::ORBITAL_LANCE_STRIKE,
            "F5",
            "SAGITTARUM AURIC VOLLEY",
            "Adrastus bolt-calivers vaporize 7M approaching organisms.",
            85.0, 0.0, true
        });
        doctrines.push_back({
            DoctrineType::EXTERMINATUS_PROTOCOL,
            "F8",
            "TALON VAULT FORBIDDEN PROTOCOL",
            "Uncages sealed archaeotech doomsday weapons to scour the realm.",
            999.0, 0.0, false
        });
    } else {
        // Imperial Standard
        doctrines.push_back({
            DoctrineType::IGNITE_PROMETHIUM_MOAT,
            "F1",
            "IGNITE PROMETHIUM MOAT",
            "Burns 2.5M gaunts, costs 15,000L fuel, repairs 12% breach.",
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
}

void CommandOrders::update(double dt, const SectorGraph& sector_graph) {
    if (dt <= 0.0) return;

    for (auto& doc : doctrines) {
        if (doc.cooldown_timer > 0.0) {
            doc.cooldown_timer = std::max(0.0, doc.cooldown_timer - dt);
        }

        if (doc.type == DoctrineType::EXTERMINATUS_PROTOCOL) {
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
    SynapseNetwork& synapse,
    IFactionRuleModule* rule_module
) {
    auto* doc = const_cast<DoctrineInfo*>(get_doctrine(type));
    if (!doc || !doc->is_available || doc->cooldown_timer > 0.0) return false;

    switch (type) {
    case DoctrineType::IGNITE_PROMETHIUM_MOAT: {
        // Slot 1: Faction-specific frontline defense / rally / surge
        if (current_faction_id == "orks") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            if (rule_module) {
                auto* ork = dynamic_cast<OrkWaaaghMechanic*>(rule_module);
                if (ork) ork->trigger_waaagh_roar();
            }
            double to_kill = 2000000.0;
            double actual_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.category == UnitCategory::INFANTRY && u.is_alive()) {
                    double killed = u.apply_direct_casualties(to_kill);
                    actual_killed += killed;
                    to_kill -= killed;
                    if (to_kill <= 0.0) break;
                }
            }
            auto& sec = sector_graph.get_active_sector();
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + sec.max_integrity * 0.10);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[WAAAGH! ROAR] GREEN MOB CRUSHED " + std::to_string((int)actual_killed) + " FOES! BARRICADE SECURED!";
            return true;
        } else if (current_faction_id == "necrons") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            if (rule_module) {
                auto* nec = dynamic_cast<NecronDynastyMechanic*>(rule_module);
                if (nec) nec->trigger_resurrection_overcharge();
            }
            double revived = 0.0;
            UnitClass* w = defenders.find_unit("necron_warrior");
            if (w) {
                double need = w->initial_count - w->count;
                double add = std::min(need, 1500000.0);
                w->count += add;
                revived += add;
            }
            last_action_message = "[REANIMATION OVERCHARGE] " + std::to_string((int)revived) + " NECRON WARRIORS RECONSTITUTED!";
            return true;
        } else if (current_faction_id == "tau" || current_faction_id == "tau_empire") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            if (rule_module) {
                auto* tau = dynamic_cast<TauEmpireMechanic*>(rule_module);
                if (tau) tau->add_markerlight_tokens(5.0);
            }
            double to_kill = 2000000.0;
            double actual_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.category == UnitCategory::INFANTRY && u.is_alive()) {
                    double killed = u.apply_direct_casualties(to_kill);
                    actual_killed += killed;
                    to_kill -= killed;
                    if (to_kill <= 0.0) break;
                }
            }
            auto& sec = sector_graph.get_active_sector();
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + sec.max_integrity * 0.08);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[MARKERLIGHT SATURATION] TARGETS DESIGNATED (+5 TOKENS)! PULSE VOLLEY DESTROYED " + std::to_string((int)actual_killed) + " INTRUDERS!";
            return true;
        } else if (current_faction_id == "death_guard") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            double to_kill = 2000000.0;
            double actual_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.category == UnitCategory::INFANTRY && u.is_alive()) {
                    double killed = u.apply_direct_casualties(to_kill);
                    actual_killed += killed;
                    to_kill -= killed;
                    if (to_kill <= 0.0) break;
                }
            }
            auto& sec = sector_graph.get_active_sector();
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + sec.max_integrity * 0.15);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[MIASMA OF PESTILENCE] CAUSTIC SLIME DISSOLVED " + std::to_string((int)actual_killed) + " FOES! BREACH SEALED!";
            return true;
        } else if (current_faction_id == "world_eaters" || current_faction_id == "khorne") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            if (rule_module) {
                auto* we = dynamic_cast<WorldEatersMechanic*>(rule_module);
                if (we) we->blood_tithe_points = std::min(120.0, we->blood_tithe_points + 25.0);
            }
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
            auto& sec = sector_graph.get_active_sector();
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + sec.max_integrity * 0.08);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[BLOOD FOR THE BLOOD GOD] BERSERKERS HACKED " + std::to_string((int)actual_killed) + " FOES! +25 BLOOD TITHE CLAIMED!";
            return true;
        } else if (current_faction_id == "sororitas" || current_faction_id == "adepta_sororitas") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            if (rule_module) {
                auto* sor = dynamic_cast<SororitasMechanic*>(rule_module);
                if (sor) {
                    sor->miracle_points = std::min(100.0, sor->miracle_points + 20.0);
                    sor->divine_shield_active = true;
                    sor->divine_shield_timer = 20.0;
                }
            }
            double to_kill = 2000000.0;
            double actual_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.category == UnitCategory::INFANTRY && u.is_alive()) {
                    double killed = u.apply_direct_casualties(to_kill);
                    actual_killed += killed;
                    to_kill -= killed;
                    if (to_kill <= 0.0) break;
                }
            }
            auto& sec = sector_graph.get_active_sector();
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + sec.max_integrity * 0.15);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[ACT OF FAITH] DIVINE AEGIS MANIFESTED! " + std::to_string((int)actual_killed) + " FOES PURGED WITH HOLY LIGHT!";
            return true;
        } else if (current_faction_id == "mechanicus" || current_faction_id == "adeptus_mechanicus") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            if (rule_module) {
                auto* mech = dynamic_cast<MechanicusMechanic*>(rule_module);
                if (mech) mech->set_protocol(CanticleProtocol::SHROUDPSALM_STEALTH);
            }
            double to_kill = 2200000.0;
            double actual_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.category == UnitCategory::INFANTRY && u.is_alive()) {
                    double killed = u.apply_direct_casualties(to_kill);
                    actual_killed += killed;
                    to_kill -= killed;
                    if (to_kill <= 0.0) break;
                }
            }
            auto& sec = sector_graph.get_active_sector();
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + sec.max_integrity * 0.12);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[BINHARIC OVERRIDE] SHROUDPSALM BROADCAST! " + std::to_string((int)actual_killed) + " FOES ATOMIZED BY GALVANIC FIRE!";
            return true;
        } else if (current_faction_id == "custodes" || current_faction_id == "adeptus_custodes") {
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;
            if (rule_module) {
                auto* cust = dynamic_cast<CustodesMechanic*>(rule_module);
                if (cust) cust->set_kata_stance(KataStance::KAPTARIS);
            }
            double to_kill = 500000.0;
            double actual_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.is_alive()) {
                    double killed = u.apply_direct_casualties(to_kill);
                    actual_killed += killed;
                    to_kill -= killed;
                    if (to_kill <= 0.0) break;
                }
            }
            auto& sec = sector_graph.get_active_sector();
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + sec.max_integrity * 0.15);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[MARTIAL KA'TAH EMBOLUS] AURAMITE PHALANX LOCKED! " + std::to_string((int)actual_killed) + " FOES SLAIN! BREACH RESTORED!";
            return true;
        } else {
            // Standard / Imperial Promethium Moat
            if (!logistics.can_afford_promethium(15000.0)) {
                last_action_message = "[ERROR] INSUFFICIENT PROMETHIUM TO IGNITE MOAT!";
                return false;
            }
            logistics.consume_promethium(15000.0);
            doc->cooldown_timer = doc->cooldown_max;
            doc->is_available = false;

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
            auto& sec = sector_graph.get_active_sector();
            double restored = sec.max_integrity * 0.12;
            sec.current_integrity = std::min(sec.max_integrity, sec.current_integrity + restored);
            sec.breach_percentage = (1.0 - (sec.current_integrity / sec.max_integrity)) * 100.0;
            last_action_message = "[ORDER EXECUTED] " + doc->name + ": " +
                std::to_string((int)actual_killed) + " FOES INCINERATED. BREACH REDUCED.";
            return true;
        }
    }

    case DoctrineType::TITAN_PLASMA_VOLLEY: {
        // Slot 2: Heavy Armor / Titan Buster
        if (current_faction_id == "imperium" || current_faction_id == "krieg") {
            if (logistics.is_plasma_overheated()) {
                last_action_message = "[ERROR] TITAN REACTORS OVERHEATED! CANNOT DISCHARGE!";
                return false;
            }
            logistics.add_plasma_heat(35.0);
        }
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;

        double heavy_kills = 0.0;
        double titan_kills = 0.0;
        auto* carn = attackers.find_unit("carnifex");
        if (carn) heavy_kills += carn->apply_direct_casualties(3500.0);
        auto* bio = attackers.find_unit("hierophant_biotitan");
        if (bio) titan_kills += bio->apply_direct_casualties(120.0);

        if (heavy_kills == 0.0 && titan_kills == 0.0) {
            for (auto& u : attackers.units) {
                if (u.is_alive()) {
                    if (u.category == UnitCategory::SUPER_HEAVY_TITAN) {
                        titan_kills += u.apply_direct_casualties(std::min(u.count, 20.0));
                    } else if (u.category == UnitCategory::ARMOR_VEHICLE) {
                        heavy_kills += u.apply_direct_casualties(std::min(u.count, 2000.0));
                    }
                }
            }
        }

        last_action_message = "[ORDER EXECUTED] " + doc->name + ": VAPORIZED " +
            std::to_string((int)heavy_kills) + " HEAVY ARMOR & " +
            std::to_string((int)titan_kills) + " TITANIC ASSETS.";
        return true;
    }

    case DoctrineType::ASTARTES_DROP_POD: {
        // Slot 3: Decapitation or Reinforcement Ingress
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;

        if (current_faction_id == "orks") {
            // Reinforce 2,000,000 Ork Boyz
            UnitClass* b = defenders.find_unit("ork_boyz");
            double added = 0.0;
            if (b) {
                double need = b->initial_count - b->count;
                added = std::min(need, 2000000.0);
                b->count += added;
            }
            last_action_message = "[GREEN TIDE MOB CALL] REINFORCED " + std::to_string((int)added) + " ORK BOYZ TO THE LINE!";
            return true;
        } else if (current_faction_id == "necrons") {
            // Summon 250,000 Immortals
            UnitClass* imm = defenders.find_unit("immortals");
            double added = 0.0;
            if (imm) {
                double need = imm->initial_count - imm->count;
                added = std::min(need, 250000.0);
                imm->count += added;
            }
            last_action_message = "[ETERNITY GATE] SUMMONED " + std::to_string((int)added) + " TESLA IMMORTALS THROUGH GATE!";
            return true;
        } else if (current_faction_id == "tau" || current_faction_id == "tau_empire") {
            UnitClass* cb = defenders.find_unit("crisis_battlesuits");
            double added = 0.0;
            if (cb) {
                double need = cb->initial_count - cb->count;
                added = std::min(need, 20000.0);
                cb->count += added;
            }
            last_action_message = "[CRISIS MANTA DROP] INSERTED " + std::to_string((int)added) + " XV8 BATTLESUITS!";
            return true;
        } else if (current_faction_id == "mechanicus" || current_faction_id == "adeptus_mechanicus") {
            UnitClass* k = defenders.find_unit("kastelan_robots");
            double added = 0.0;
            if (k) {
                double need = k->initial_count - k->count;
                added = std::min(need, 5000.0);
                k->count += added;
            }
            last_action_message = "[CYBERNETICA PULSE] AWAKENED " + std::to_string((int)added) + " KASTELAN ROBOT COHORTS!";
            return true;
        } else if (current_faction_id == "custodes" || current_faction_id == "adeptus_custodes") {
            UnitClass* allarus = defenders.find_unit("allarus_terminator");
            double added = 0.0;
            if (allarus) {
                double need = allarus->initial_count - allarus->count;
                added = std::min(need, 1000.0);
                allarus->count += added;
            }
            double cmd_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.is_alive() && (u.is_synapse || u.category == UnitCategory::HEAVY_INFANTRY)) {
                    cmd_killed += u.apply_direct_casualties(std::min(u.count, 5000.0));
                    break;
                }
            }
            last_action_message = "[ALLARUS SHADOWKEEP INGRESS] REINFORCED " + std::to_string((int)added) +
                " ALLARUS TERMINATORS & DECAPITATED " + std::to_string((int)cmd_killed) + " COMMANDERS!";
            return true;
        } else if (current_faction_id == "death_guard") {
            UnitClass* ds = defenders.find_unit("deathshroud");
            double added = 0.0;
            if (ds) {
                double need = ds->initial_count - ds->count;
                added = std::min(need, 5000.0);
                ds->count += added;
            }
            double cmd_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.is_alive() && (u.is_synapse || u.category == UnitCategory::HEAVY_INFANTRY)) {
                    cmd_killed += u.apply_direct_casualties(std::min(u.count, 5000.0));
                    break;
                }
            }
            last_action_message = "[DEATHSHROUD SPEARHEAD] TELEPORTED " + std::to_string((int)added) +
                " DEATHSHROUD & SLAIN " + std::to_string((int)cmd_killed) + " SYNAPTICS!";
            return true;
        } else if (current_faction_id == "world_eaters" || current_faction_id == "khorne") {
            UnitClass* eb = defenders.find_unit("eightbound");
            double added = 0.0;
            if (eb) {
                double need = eb->initial_count - eb->count;
                added = std::min(need, 15000.0);
                eb->count += added;
            }
            double cmd_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.is_alive() && (u.is_synapse || u.category == UnitCategory::HEAVY_INFANTRY)) {
                    cmd_killed += u.apply_direct_casualties(std::min(u.count, 5000.0));
                    break;
                }
            }
            last_action_message = "[RED BUTCHER ASSAULT] DEPLOYED " + std::to_string((int)added) +
                " EIGHTBOUND & CRUSHED " + std::to_string((int)cmd_killed) + " COMMANDERS!";
            return true;
        } else if (current_faction_id == "sororitas" || current_faction_id == "adepta_sororitas") {
            UnitClass* ret = defenders.find_unit("retributors");
            double added = 0.0;
            if (ret) {
                double need = ret->initial_count - ret->count;
                added = std::min(need, 10000.0);
                ret->count += added;
            }
            double cmd_killed = 0.0;
            for (auto& u : attackers.units) {
                if (u.is_alive() && (u.is_synapse || u.category == UnitCategory::HEAVY_INFANTRY)) {
                    cmd_killed += u.apply_direct_casualties(std::min(u.count, 5000.0));
                    break;
                }
            }
            last_action_message = "[SERAPHIM CELESTIAL DESCENT] INSERTED " + std::to_string((int)added) +
                " RETRIBUTORS & PURGED " + std::to_string((int)cmd_killed) + " SYNAPTIC LEADERS!";
            return true;
        } else {
            // Decapitation strike on commanders / synapse nodes
            if (current_faction_id == "imperium" && logistics.bolter_rounds < 25000.0) {
                last_action_message = "[ERROR] INSUFFICIENT BOLTER AMMUNITION FOR DROP PODS!";
                return false;
            }
            if (current_faction_id == "imperium") {
                logistics.consume_ammo(AmmoType::AMMO_BOLTER_ROUNDS, 25000.0);
            }

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
            last_action_message = "[ORDER EXECUTED] " + doc->name + ": ELIMINATED " +
                std::to_string((int)(warriors_killed + zoans_killed)) + " PRIORITY COMMANDERS. ENEMY SHOCKED!";
            return true;
        }
    }

    case DoctrineType::TACTICAL_FALLBACK: {
        if (sector_graph.active_sector_index >= sector_graph.sectors.size() - 1) {
            last_action_message = "[ERROR] CITADEL IS FINAL SECTOR. NO FURTHER RETREAT POSSIBLE!";
            return false;
        }
        doc->cooldown_timer = doc->cooldown_max;
        doc->is_available = false;
        sector_graph.fallback_to_next_sector();
        last_action_message = "[ORDER EXECUTED] " + doc->name + " TO " +
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
        for (auto& u : defenders.units) {
            if (u.is_alive() && u.category == UnitCategory::INFANTRY) {
                collateral += u.apply_direct_casualties(std::min(u.count, 40000.0));
                break;
            }
        }

        last_action_message = "[ORDER EXECUTED] " + doc->name + ": " +
            std::to_string((int)actual_swarm) + " ENEMY KILLS. " +
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
        last_action_message = "[CRITICAL] " + doc->name + " DETONATED. PLANET PURGED.";
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
    configure_for_faction(current_faction_id.empty() ? "imperium" : current_faction_id);
    for (auto& doc : doctrines) {
        doc.cooldown_timer = 0.0;
        doc.is_available = (doc.type != DoctrineType::EXTERMINATUS_PROTOCOL);
    }
}

} // namespace omnisbelli

#include "engine/faction_rule_module.hpp"
#include "engine/army_roster.hpp"
#include "engine/synapse_network.hpp"
#include "engine/death_guard_mechanic.hpp"
#include "engine/sororitas_mechanic.hpp"
#include "engine/mechanicus_mechanic.hpp"
#include "engine/world_eaters_mechanic.hpp"
#include "engine/ork_waaagh_mechanic.hpp"
#include "engine/necron_dynasty_mechanic.hpp"
#include "engine/tau_empire_mechanic.hpp"
#include "engine/custodes_mechanic.hpp"
#include <algorithm>

namespace omnisbelli {

GenericFactionModule::GenericFactionModule(const std::string& fid, const std::string& title)
    : faction_id(fid), display_title(title) {}

std::string GenericFactionModule::get_status_description() const {
    return display_title + " // COHESION NOMINAL";
}

void GenericFactionModule::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)friendly;
    (void)enemy;
    (void)active_sector;
    (void)dt;
}

TyranidSynapseModule::TyranidSynapseModule(const SynapseNetwork& net)
    : synapse(net) {}

std::string TyranidSynapseModule::get_status_description() const {
    return synapse.get_status_description();
}

std::string TyranidSynapseModule::get_telemetry_tag() const {
    return synapse.get_state_tag();
}

double TyranidSynapseModule::get_telemetry_ratio() const {
    return std::clamp(synapse.synapse_ratio, 0.0, 1.0);
}

void TyranidSynapseModule::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)enemy;
    (void)active_sector;
    synapse.update(friendly, dt);
}

void TyranidSynapseModule::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    out_damage.raw_damage *= synapse.get_attack_multiplier();
}

double TyranidSynapseModule::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    // Instinctive feral swarm suffers up to 2.0x casualties from panic
    return incoming_raw_damage * synapse.get_casualty_multiplier();
}

void TyranidSynapseModule::reset() {
    synapse.reset();
}

std::unique_ptr<IFactionRuleModule> create_faction_rule_module(const std::string& faction_id) {
    if (faction_id == "tyranids") {
        return std::make_unique<TyranidSynapseModule>();
    } else if (faction_id == "death_guard") {
        return std::make_unique<DeathGuardMechanic>();
    } else if (faction_id == "adepta_sororitas" || faction_id == "sororitas") {
        return std::make_unique<SororitasMechanic>();
    } else if (faction_id == "adeptus_mechanicus" || faction_id == "mechanicus") {
        return std::make_unique<MechanicusMechanic>();
    } else if (faction_id == "world_eaters" || faction_id == "khorne") {
        return std::make_unique<WorldEatersMechanic>();
    } else if (faction_id == "imperium") {
        return std::make_unique<GenericFactionModule>("imperium", "IMPERIAL COGITATOR BATTLEGRID");
    } else if (faction_id == "orks") {
        return std::make_unique<OrkWaaaghMechanic>();
    } else if (faction_id == "necrons") {
        return std::make_unique<NecronDynastyMechanic>();
    } else if (faction_id == "tau" || faction_id == "tau_empire") {
        return std::make_unique<TauEmpireMechanic>();
    } else if (faction_id == "custodes" || faction_id == "adeptus_custodes") {
        return std::make_unique<CustodesMechanic>();
    } else if (faction_id == "aeldari" || faction_id == "eldar") {
        return std::make_unique<GenericFactionModule>("aeldari", "ASURYANI WEBWAY MESH");
    } else if (faction_id == "krieg") {
        return std::make_unique<GenericFactionModule>("krieg", "KRIEG NO-STEP-BACK ATTRITION");
    } else if (faction_id == "iron_warriors") {
        return std::make_unique<GenericFactionModule>("iron_warriors", "SIEGE BREAKER REDOUBT PROTOCOL");
    } else if (faction_id == "chaos") {
        return std::make_unique<GenericFactionModule>("chaos", "WARP INSTABILITY INCURSION");
    } else if (faction_id == "gsc") {
        return std::make_unique<GenericFactionModule>("gsc", "CULT SUBTERRANEAN AMBUSH");
    }
    return std::make_unique<GenericFactionModule>(faction_id, "COMBAT NETWORK ACTIVE");
}

} // namespace omnisbelli

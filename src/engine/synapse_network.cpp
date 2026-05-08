#include "engine/synapse_network.hpp"
#include "engine/army_roster.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

SynapseNetwork::SynapseNetwork()
    : living_capacity(1.0), required_capacity(1.0), synapse_ratio(1.0),
      current_state(SynapseCoherenceState::COORDINATED),
      time_in_feral_state(0.0), total_feral_infighting_casualties(0.0) {}

void SynapseNetwork::update(ArmyRoster& swarm_roster, double dt) {
    living_capacity = swarm_roster.get_living_synapse_capacity();
    required_capacity = swarm_roster.get_required_synapse_capacity();

    if (required_capacity <= 0.0) {
        synapse_ratio = 1.0;
    } else {
        synapse_ratio = living_capacity / required_capacity;
    }

    // Determine state
    if (synapse_ratio >= 0.50) {
        current_state = SynapseCoherenceState::COORDINATED;
    } else if (synapse_ratio >= 0.25) {
        current_state = SynapseCoherenceState::DISRUPTED;
    } else {
        current_state = SynapseCoherenceState::INSTINCTIVE_FERAL;
        time_in_feral_state += dt;

        // Feral infighting: gaunts panic, cannibalize each other, and stampede
        // 0.2% per second of living non-synapse biomass dies to infighting
        double infighting_rate = 0.002 * dt;
        for (auto& u : swarm_roster.units) {
            if (!u.is_synapse && u.is_alive()) {
                double kills = u.count * infighting_rate;
                double actual = u.apply_direct_casualties(kills);
                total_feral_infighting_casualties += actual;
            }
        }
    }
}

double SynapseNetwork::get_attack_multiplier() const {
    switch (current_state) {
    case SynapseCoherenceState::COORDINATED:
        return 1.0;
    case SynapseCoherenceState::DISRUPTED:
        return 0.70;
    case SynapseCoherenceState::INSTINCTIVE_FERAL:
        return 0.30; // 70% damage reduction due to lack of coordination
    default:
        return 1.0;
    }
}

double SynapseNetwork::get_casualty_multiplier() const {
    switch (current_state) {
    case SynapseCoherenceState::COORDINATED:
        return 1.0;
    case SynapseCoherenceState::DISRUPTED:
        return 1.30;
    case SynapseCoherenceState::INSTINCTIVE_FERAL:
        return 2.00; // Double casualties from mindless charging into fire funnels
    default:
        return 1.0;
    }
}

std::string SynapseNetwork::get_status_description() const {
    switch (current_state) {
    case SynapseCoherenceState::COORDINATED:
        return "SYNAPTIC WEB COHERENT // 100% LETHALITY";
    case SynapseCoherenceState::DISRUPTED:
        return "SYNAPTIC WEB FLUCTUATING // 70% LETHALITY";
    case SynapseCoherenceState::INSTINCTIVE_FERAL:
        return "CRITICAL DISRUPTION: INSTINCTIVE BEHAVIOR ACTIVE (FERAL)";
    default:
        return "UNKNOWN";
    }
}

std::string SynapseNetwork::get_state_tag() const {
    switch (current_state) {
    case SynapseCoherenceState::COORDINATED:
        return "SYNAPSE: OPTIMAL";
    case SynapseCoherenceState::DISRUPTED:
        return "SYNAPSE: DISRUPTED";
    case SynapseCoherenceState::INSTINCTIVE_FERAL:
        return "SYNAPSE: COLLAPSE";
    default:
        return "SYNAPSE: N/A";
    }
}

bool SynapseNetwork::is_in_instinctive_frenzy() const {
    return current_state == SynapseCoherenceState::INSTINCTIVE_FERAL;
}

void SynapseNetwork::reset() {
    living_capacity = 1.0;
    required_capacity = 1.0;
    synapse_ratio = 1.0;
    current_state = SynapseCoherenceState::COORDINATED;
    time_in_feral_state = 0.0;
    total_feral_infighting_casualties = 0.0;
}

} // namespace omnisbelli

#pragma once
#include <string>

namespace omnisbelli {

class ArmyRoster;

enum class SynapseCoherenceState {
    COORDINATED,        // ratio >= 0.50: Full Hive Mind lethality
    DISRUPTED,          // 0.25 <= ratio < 0.50: Degraded coordination
    INSTINCTIVE_FERAL   // ratio < 0.25: Feral collapse, cannibalism, mindless charges
};

class SynapseNetwork {
public:
    double living_capacity;
    double required_capacity;
    double synapse_ratio;

    SynapseCoherenceState current_state;
    double time_in_feral_state;
    double total_feral_infighting_casualties;

    SynapseNetwork();

    void update(ArmyRoster& swarm_roster, double dt);

    double get_attack_multiplier() const;
    double get_casualty_multiplier() const;
    std::string get_status_description() const;
    std::string get_state_tag() const;

    bool is_in_instinctive_frenzy() const;
    void reset();
};

} // namespace omnisbelli

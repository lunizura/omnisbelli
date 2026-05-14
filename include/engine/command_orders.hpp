#pragma once
#include "engine/army_roster.hpp"
#include "engine/logistics.hpp"
#include "engine/sector.hpp"
#include "engine/synapse_network.hpp"
#include <string>
#include <vector>

namespace omnisbelli {

enum class DoctrineType {
    IGNITE_PROMETHIUM_MOAT,     // F1
    TITAN_PLASMA_VOLLEY,        // F2
    ASTARTES_DROP_POD,          // F3
    TACTICAL_FALLBACK,          // F4
    ORBITAL_LANCE_STRIKE,       // F5
    EXTERMINATUS_PROTOCOL       // F8
};

struct DoctrineInfo {
    DoctrineType type;
    std::string key_binding;
    std::string name;
    std::string description;
    double cooldown_max;
    double cooldown_timer;
    bool is_available;
};

class CommandOrders {
public:
    std::vector<DoctrineInfo> doctrines;
    bool exterminatus_triggered;
    std::string last_action_message;

    CommandOrders();

    void update(double dt, const SectorGraph& sector_graph);

    bool trigger_doctrine(
        DoctrineType type,
        ArmyRoster& defenders,
        ArmyRoster& attackers,
        SectorGraph& sector_graph,
        LogisticsSystem& logistics,
        SynapseNetwork& synapse
    );

    const DoctrineInfo* get_doctrine(DoctrineType type) const;
    void reset();
};

} // namespace omnisbelli

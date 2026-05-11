#include "engine/sector.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

Sector::Sector()
    : id(0), name("Unknown Sector"), designation("SEC-0"),
      description(""), chokepoint_width(10000.0),
      fortification_cover(0.2), max_integrity(10000000.0),
      current_integrity(10000000.0), breach_percentage(0.0),
      status(SectorStatus::SECURE) {}

Sector::Sector(
    int id,
    const std::string& name,
    const std::string& designation,
    const std::string& description,
    double chokepoint_width,
    double fortification_cover,
    double max_integrity
)
    : id(id), name(name), designation(designation),
      description(description),
      chokepoint_width(std::max(100.0, chokepoint_width)),
      fortification_cover(std::clamp(fortification_cover, 0.0, 0.90)),
      max_integrity(std::max(1000.0, max_integrity)),
      current_integrity(max_integrity),
      breach_percentage(0.0),
      status(SectorStatus::SECURE) {}

double Sector::apply_siege_damage(double raw_damage) {
    if (status == SectorStatus::FALLEN || raw_damage <= 0.0) return 0.0;

    current_integrity -= raw_damage;
    if (current_integrity <= 0.0) {
        current_integrity = 0.0;
        breach_percentage = 100.0;
        status = SectorStatus::FALLEN;
    } else {
        breach_percentage = (1.0 - (current_integrity / max_integrity)) * 100.0;
        if (breach_percentage >= 70.0) {
            status = SectorStatus::CRITICAL;
        } else {
            status = SectorStatus::CONTESTED;
        }
    }
    return breach_percentage;
}

double Sector::get_chokepoint_ratio(double total_attacker_footprint) const {
    if (total_attacker_footprint <= 0.0) return 1.0;
    return std::min(1.0, chokepoint_width / total_attacker_footprint);
}

bool Sector::is_fallen() const {
    return status == SectorStatus::FALLEN;
}

void Sector::reset() {
    current_integrity = max_integrity;
    breach_percentage = 0.0;
    status = SectorStatus::SECURE;
}

SectorGraph::SectorGraph()
    : active_sector_index(0) {}

Sector& SectorGraph::get_active_sector() {
    if (active_sector_index >= sectors.size()) {
        return sectors.back();
    }
    return sectors[active_sector_index];
}

const Sector& SectorGraph::get_active_sector() const {
    if (active_sector_index >= sectors.size()) {
        return sectors.back();
    }
    return sectors[active_sector_index];
}

bool SectorGraph::advance_active_sector() {
    if (active_sector_index + 1 < sectors.size()) {
        sectors[active_sector_index].status = SectorStatus::FALLEN;
        sectors[active_sector_index].breach_percentage = 100.0;
        sectors[active_sector_index].current_integrity = 0.0;
        active_sector_index++;
        sectors[active_sector_index].status = SectorStatus::CONTESTED;
        return true;
    }
    sectors[active_sector_index].status = SectorStatus::FALLEN;
    return false; // Citadel has fallen, game over
}

bool SectorGraph::fallback_to_next_sector() {
    if (active_sector_index + 1 < sectors.size()) {
        sectors[active_sector_index].status = SectorStatus::FALLEN;
        sectors[active_sector_index].breach_percentage = 100.0;
        sectors[active_sector_index].current_integrity = 0.0;
        active_sector_index++;
        sectors[active_sector_index].status = SectorStatus::CONTESTED;
        return true;
    }
    return false;
}

bool SectorGraph::is_all_fallen() const {
    return active_sector_index >= sectors.size() - 1 && sectors.back().status == SectorStatus::FALLEN;
}

void SectorGraph::reset() {
    // Initialize active sector index to 0
    active_sector_index = 0;
    for (size_t i = 0; i < sectors.size(); i++) {
        sectors[i].reset();
    }
    if (!sectors.empty()) {
        sectors[0].status = SectorStatus::CONTESTED;
    }
}

SectorGraph SectorGraph::create_standard_planetary_graph() {
    SectorGraph graph;

    // Sector 1: Orbital Grid
    graph.sectors.emplace_back(
        1, "Orbital Defense Grid", "SEC-ORBIT",
        "Orbital defense laser platforms and macro-torpedo stations.",
        10000000.0, 0.05, 80000000.0
    );

    // Sector 2: Ash Wastes
    graph.sectors.emplace_back(
        2, "Ash Wastes Perimeter", "SEC-WASTES",
        "Open sulfur plains and minefields. Pure Lanchester Square Law.",
        350000.0, 0.20, 120000000.0
    );

    // Sector 3: Trench Line Alpha
    graph.sectors.emplace_back(
        3, "Trench Line Alpha", "SEC-TRENCH",
        "Subterranean bunkers, razorwire, and promethium moats.",
        45000.0, 0.50, 180000000.0
    );

    // Sector 4: Bastion Iron Gates
    graph.sectors.emplace_back(
        4, "Bastion Iron Gates", "SEC-BASTION",
        "Adamantium curtain fortress walls. Severe chokepoint funnel.",
        5000.0, 0.75, 250000000.0
    );

    // Sector 5: Hive Sub-Levels
    graph.sectors.emplace_back(
        5, "Hive Sub-Levels", "SEC-SUBLEVEL",
        "Claustrophobic manufactorum tunnels and sewer catacombs.",
        1200.0, 0.60, 200000000.0
    );

    // Sector 6: Spire Citadel
    graph.sectors.emplace_back(
        6, "Spire Citadel & Cogitator", "SEC-CITADEL",
        "The Lord General command sanctum. Planetary Cogitator heart.",
        400.0, 0.70, 350000000.0
    );

    graph.sectors[0].status = SectorStatus::CONTESTED;
    return graph;
}

} // namespace omnisbelli

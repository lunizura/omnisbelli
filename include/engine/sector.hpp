#pragma once
#include <string>
#include <vector>
#include <algorithm>

namespace omnisbelli {

enum class SectorStatus {
    SECURE,
    CONTESTED,
    CRITICAL,
    FALLEN
};

class Sector {
public:
    int id;
    std::string name;
    std::string designation;
    std::string description;

    double chokepoint_width;        // Width in meters
    double fortification_cover;     // [0.0, 0.85] damage reduction for defenders
    double max_integrity;
    double current_integrity;
    double breach_percentage;       // [0.0, 100.0]%
    SectorStatus status;

    Sector();
    Sector(
        int id,
        const std::string& name,
        const std::string& designation,
        const std::string& description,
        double chokepoint_width,
        double fortification_cover,
        double max_integrity
    );

    double apply_siege_damage(double raw_damage);
    double get_chokepoint_ratio(double total_attacker_footprint) const;

    bool is_fallen() const;
    void reset();
};

class SectorGraph {
public:
    std::vector<Sector> sectors;
    size_t active_sector_index;

    SectorGraph();

    Sector& get_active_sector();
    const Sector& get_active_sector() const;

    bool advance_active_sector();
    bool fallback_to_next_sector();
    bool is_all_fallen() const;
    void reset();

    static SectorGraph create_standard_planetary_graph();
};

} // namespace omnisbelli

#pragma once
#include "engine/simulation.hpp"
#include <string>

namespace omnisbelli {

class ConfigLoader {
public:
    // Load simulation configuration and unit parameters from a JSON file
    static bool load_simulation_config(const std::string& filepath, SimulationEngine& sim);

    // Export current simulation state and army rosters to a JSON config file
    static bool save_simulation_config(const std::string& filepath, const SimulationEngine& sim);

    // Lightweight zero-dependency JSON token extraction helpers
    static bool extract_json_string(const std::string& json, const std::string& key, std::string& out_val);
    static bool extract_json_number(const std::string& json, const std::string& key, double& out_val);
    static bool extract_json_int(const std::string& json, const std::string& key, int& out_val);
};

} // namespace omnisbelli

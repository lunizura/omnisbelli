#include "engine/config_loader.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace omnisbelli {

bool ConfigLoader::extract_json_string(const std::string& json, const std::string& key, std::string& out_val) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;

    size_t colon = json.find(':', pos + needle.length());
    if (colon == std::string::npos) return false;

    for (size_t i = pos + needle.length(); i < colon; ++i) {
        if (!std::isspace(static_cast<unsigned char>(json[i]))) return false;
    }

    size_t val_start = colon + 1;
    while (val_start < json.length() && std::isspace(static_cast<unsigned char>(json[val_start]))) {
        val_start++;
    }
    if (val_start >= json.length() || json[val_start] != '\"') return false;

    size_t close_quote = json.find('\"', val_start + 1);
    if (close_quote == std::string::npos) return false;

    out_val = json.substr(val_start + 1, close_quote - val_start - 1);
    return true;
}

bool ConfigLoader::extract_json_number(const std::string& json, const std::string& key, double& out_val) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;

    size_t colon = json.find(':', pos + needle.length());
    if (colon == std::string::npos) return false;

    for (size_t i = pos + needle.length(); i < colon; ++i) {
        if (!std::isspace(static_cast<unsigned char>(json[i]))) return false;
    }

    size_t val_start = colon + 1;
    while (val_start < json.length() && std::isspace(static_cast<unsigned char>(json[val_start]))) {
        val_start++;
    }
    if (val_start >= json.length()) return false;

    // Disallow strings, arrays, or objects
    if (json[val_start] == '\"' || json[val_start] == '{' || json[val_start] == '[') return false;

    size_t val_end = val_start;
    while (val_end < json.length() && (std::isdigit(static_cast<unsigned char>(json[val_end])) ||
                                      json[val_end] == '.' ||
                                      json[val_end] == '-' ||
                                      json[val_end] == '+' ||
                                      json[val_end] == 'e' ||
                                      json[val_end] == 'E')) {
        val_end++;
    }

    if (val_end > val_start) {
        try {
            out_val = std::stod(json.substr(val_start, val_end - val_start));
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

bool ConfigLoader::extract_json_int(const std::string& json, const std::string& key, int& out_val) {
    double d = 0.0;
    if (extract_json_number(json, key, d)) {
        out_val = static_cast<int>(std::round(d));
        return true;
    }
    return false;
}

bool ConfigLoader::load_simulation_config(const std::string& filepath, SimulationEngine& sim) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    if (content.empty()) return false;

    // 1. Faction Overrides
    std::string def_faction;
    if (extract_json_string(content, "defender_faction", def_faction)) {
        sim.set_defender_faction(def_faction);
    }

    std::string att_faction;
    if (extract_json_string(content, "attacker_faction", att_faction)) {
        sim.set_attacker_faction(att_faction);
    }

    // 2. Simulation Speed & Active Sector
    double speed = 1.0;
    if (extract_json_number(content, "time_multiplier", speed)) {
        sim.set_speed(speed);
    }

    int sector_idx = 0;
    if (extract_json_int(content, "active_sector_index", sector_idx)) {
        if (sector_idx >= 0 && sector_idx < static_cast<int>(sim.sector_graph.sectors.size())) {
            sim.sector_graph.active_sector_index = static_cast<size_t>(sector_idx);
        }
    }

    // 3. Logistics Reserves
    double laspacks = 0.0;
    if (extract_json_number(content, "laspack_cells", laspacks) || extract_json_number(content, "laspack_mwh", laspacks)) {
        sim.logistics.laspack_mwh = laspacks;
        sim.logistics.laspack_max_mwh = std::max(sim.logistics.laspack_max_mwh, laspacks);
    }

    double bolters = 0.0;
    if (extract_json_number(content, "bolter_rounds", bolters)) {
        sim.logistics.bolter_rounds = bolters;
        sim.logistics.bolter_max_rounds = std::max(sim.logistics.bolter_max_rounds, bolters);
    }

    double artillery = 0.0;
    if (extract_json_number(content, "artillery_shells", artillery)) {
        sim.logistics.artillery_shells = artillery;
        sim.logistics.artillery_max_shells = std::max(sim.logistics.artillery_max_shells, artillery);
    }

    double promethium = 0.0;
    if (extract_json_number(content, "promethium_liters", promethium)) {
        sim.logistics.promethium_liters = promethium;
        sim.logistics.promethium_max_liters = std::max(sim.logistics.promethium_max_liters, promethium);
    }

    // 4. Unit Overrides
    size_t units_pos = content.find("\"units\"");
    if (units_pos != std::string::npos) {
        size_t arr_open = content.find('[', units_pos);
        size_t arr_close = content.find(']', arr_open);
        if (arr_open != std::string::npos && arr_close != std::string::npos) {
            std::string arr_str = content.substr(arr_open, arr_close - arr_open + 1);

            size_t obj_start = 0;
            while ((obj_start = arr_str.find('{', obj_start)) != std::string::npos) {
                size_t obj_end = arr_str.find('}', obj_start);
                if (obj_end == std::string::npos) break;

                std::string obj_str = arr_str.substr(obj_start, obj_end - obj_start + 1);

                std::string uid;
                if (extract_json_string(obj_str, "id", uid)) {
                    UnitClass* u = sim.defenders.find_unit(uid);
                    if (!u) u = sim.attackers.find_unit(uid);

                    if (u) {
                        double count = 0.0;
                        if (extract_json_number(obj_str, "count", count)) {
                            u->initial_count = count;
                            u->count = count;
                        }
                        double hp = 0.0;
                        if (extract_json_number(obj_str, "hp", hp)) {
                            u->hp_per_unit = hp;
                        }
                        double armor = 0.0;
                        if (extract_json_number(obj_str, "armor", armor)) {
                            u->armor = std::clamp(armor, 0.0, 0.95);
                        }
                        double dps = 0.0;
                        if (extract_json_number(obj_str, "dps", dps)) {
                            u->dps_per_unit = dps;
                        }
                    }
                }
                obj_start = obj_end + 1;
            }
        }
    }

    sim.run_planetary_projection(20);
    sim.add_event("[CONFIG LOADED] Configuration successfully applied from: " + filepath);
    return true;
}

bool ConfigLoader::save_simulation_config(const std::string& filepath, const SimulationEngine& sim) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "{\n";
    file << "  \"engine\": \"OMNISBELLI v1.0.0\",\n";
    file << "  \"defender_faction\": \"" << sim.defenders.faction_id << "\",\n";
    file << "  \"attacker_faction\": \"" << sim.attackers.faction_id << "\",\n";
    file << "  \"time_multiplier\": " << std::fixed << std::setprecision(1) << sim.time_multiplier << ",\n";
    file << "  \"active_sector_index\": " << sim.sector_graph.active_sector_index << ",\n";
    file << "  \"logistics\": {\n";
    file << "    \"laspack_mwh\": " << std::fixed << std::setprecision(0) << sim.logistics.laspack_mwh << ",\n";
    file << "    \"bolter_rounds\": " << std::fixed << std::setprecision(0) << sim.logistics.bolter_rounds << ",\n";
    file << "    \"artillery_shells\": " << std::fixed << std::setprecision(0) << sim.logistics.artillery_shells << ",\n";
    file << "    \"promethium_liters\": " << std::fixed << std::setprecision(0) << sim.logistics.promethium_liters << "\n";
    file << "  },\n";
    file << "  \"units\": [\n";

    bool first_unit = true;
    auto write_unit = [&](const UnitClass& u) {
        if (!first_unit) file << ",\n";
        first_unit = false;
        file << "    {\n";
        file << "      \"id\": \"" << u.id << "\",\n";
        file << "      \"name\": \"" << u.name << "\",\n";
        file << "      \"faction_id\": \"" << u.faction_id << "\",\n";
        file << "      \"count\": " << std::fixed << std::setprecision(0) << u.count << ",\n";
        file << "      \"hp\": " << std::fixed << std::setprecision(1) << u.hp_per_unit << ",\n";
        file << "      \"armor\": " << std::fixed << std::setprecision(2) << u.armor << ",\n";
        file << "      \"dps\": " << std::fixed << std::setprecision(1) << u.dps_per_unit << "\n";
        file << "    }";
    };

    for (const auto& u : sim.defenders.units) {
        write_unit(u);
    }
    for (const auto& u : sim.attackers.units) {
        write_unit(u);
    }

    file << "\n  ]\n";
    file << "}\n";
    return true;
}

} // namespace omnisbelli

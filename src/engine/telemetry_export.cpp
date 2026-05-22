#include "engine/telemetry_export.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace omnisbelli {

bool TelemetryExport::export_battle_json(const std::string& filepath,
                                         const ArmyRoster& def,
                                         const ArmyRoster& att,
                                         const Sector& sector,
                                         const LogisticsSystem& log,
                                         double elapsed_time_seconds,
                                         const std::string& outcome) {
    std::ofstream f(filepath);
    if (!f.is_open()) return false;

    f << "{\n";
    f << "  \"engine\": \"OMNISBELLI v1.0.0\",\n";
    f << "  \"elapsed_days\": " << std::fixed << std::setprecision(2) << (elapsed_time_seconds / 86400.0) << ",\n";
    f << "  \"outcome\": \"" << outcome << "\",\n";
    f << "  \"active_sector\": \"" << sector.name << "\",\n";
    f << "  \"sector_breach_pct\": " << sector.breach_percentage << ",\n";
    f << "  \"defenders\": {\n";
    f << "    \"name\": \"" << def.display_name << "\",\n";
    f << "    \"initial_count\": " << (uint64_t)def.get_total_initial_count() << ",\n";
    f << "    \"living_count\": " << (uint64_t)def.get_total_count() << ",\n";
    f << "    \"casualties\": " << (uint64_t)def.get_total_casualties() << "\n";
    f << "  },\n";
    f << "  \"invaders\": {\n";
    f << "    \"name\": \"" << att.display_name << "\",\n";
    f << "    \"initial_count\": " << (uint64_t)att.get_total_initial_count() << ",\n";
    f << "    \"living_count\": " << (uint64_t)att.get_total_count() << ",\n";
    f << "    \"casualties\": " << (uint64_t)att.get_total_casualties() << "\n";
    f << "  },\n";
    f << "  \"munitions\": {\n";
    f << "    \"laspack_power_pct\": " << (log.get_reserve_percentage(AmmoType::AMMO_LASPACKS) * 100.0) << ",\n";
    f << "    \"bolter_ammo_pct\": " << (log.get_reserve_percentage(AmmoType::AMMO_BOLTER_ROUNDS) * 100.0) << ",\n";
    f << "    \"artillery_shells_pct\": " << (log.get_reserve_percentage(AmmoType::AMMO_ARTILLERY_SHELLS) * 100.0) << ",\n";
    f << "    \"promethium_liters\": " << log.promethium_liters << ",\n";
    f << "    \"titan_plasma_heat_pct\": " << log.titan_plasma_heat << "\n";
    f << "  }\n";
    f << "}\n";
    return true;
}

bool TelemetryExport::export_forensic_log(const std::string& filepath,
                                          const ArmyRoster& def,
                                          const ArmyRoster& att,
                                          const Sector& sector,
                                          const LogisticsSystem& log,
                                          double elapsed_time_seconds,
                                          const std::string& outcome) {
    std::ofstream f(filepath);
    if (!f.is_open()) return false;

    f << "================================================================================\n";
    f << " ADEPTUS MECHANICUS // EXPEDITION BIO-FORENSIC DISPATCH // OMNISBELLI ENGINE   \n";
    f << "================================================================================\n\n";
    f << " [BATTLE OUTCOME: " << outcome << "]\n";
    f << " Elapsed Duration   : Day " << std::fixed << std::setprecision(1) << (elapsed_time_seconds / 86400.0) << "\n";
    f << " Active Sector      : " << sector.name << " (Breach: " << (int)sector.breach_percentage << "%)\n\n";
    f << " [DEFENDING FORCE: " << def.display_name << "]\n";
    f << "  Initial Garrison   : " << (uint64_t)def.get_total_initial_count() << "\n";
    f << "  Active Survivors   : " << (uint64_t)def.get_total_count() << "\n";
    f << "  Casualties Suffered: " << (uint64_t)def.get_total_casualties() << " (" 
      << std::setprecision(1) << (100.0 - def.get_survival_percentage()) << "%)\n\n";
    f << " [INVADING HOST: " << att.display_name << "]\n";
    f << "  Initial Mass       : " << (uint64_t)att.get_total_initial_count() << "\n";
    f << "  Remaining Host     : " << (uint64_t)att.get_total_count() << "\n";
    f << "  Eliminated Host    : " << (uint64_t)att.get_total_casualties() << " ("
      << std::setprecision(1) << (100.0 - att.get_survival_percentage()) << "%)\n\n";
    f << " [MUNITIONS EXPENDITURE]\n";
    f << "  Laspack MWh Level  : " << (int)(log.get_reserve_percentage(AmmoType::AMMO_LASPACKS) * 100.0) << "%\n";
    f << "  Artillery Shells   : " << (int)(log.get_reserve_percentage(AmmoType::AMMO_ARTILLERY_SHELLS) * 100.0) << "%\n";
    f << "  Promethium Fuel    : " << (uint64_t)log.promethium_liters << " Liters\n\n";
    f << " OMNISSIAH RECORD VERIFIED. DISPATCH ARCHIVED.\n";
    f << "================================================================================\n";
    return true;
}

} // namespace omnisbelli

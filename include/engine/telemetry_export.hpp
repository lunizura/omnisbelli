#pragma once
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include "engine/logistics.hpp"
#include <string>

namespace omnisbelli {

class TelemetryExport {
public:
    static bool export_battle_json(const std::string& filepath,
                                   const ArmyRoster& def,
                                   const ArmyRoster& att,
                                   const Sector& sector,
                                   const LogisticsSystem& log,
                                   double elapsed_time_seconds,
                                   const std::string& outcome);

    static bool export_forensic_log(const std::string& filepath,
                                    const ArmyRoster& def,
                                    const ArmyRoster& att,
                                    const Sector& sector,
                                    const LogisticsSystem& log,
                                    double elapsed_time_seconds,
                                    const std::string& outcome);
};

} // namespace omnisbelli

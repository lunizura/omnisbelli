#pragma once
#include <string>
#include <vector>
#include <memory>

namespace omnisbelli {

class ArmyRoster;
class Sector;

class IFactionRuleModule {
public:
    virtual ~IFactionRuleModule() = default;

    virtual std::string get_faction_id() const = 0;
    virtual std::string get_status_description() const = 0;
    virtual std::string get_telemetry_tag() const = 0;
    virtual double get_telemetry_ratio() const = 0;

    virtual void pre_combat_tick(
        ArmyRoster& friendly,
        ArmyRoster& enemy,
        Sector& active_sector,
        double dt
    ) = 0;

    virtual void reset() = 0;
};

} // namespace omnisbelli

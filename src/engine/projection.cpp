#include "engine/projection.hpp"
#include "engine/lanchester.hpp"
#include "engine/synapse_network.hpp"
#include <random>
#include <algorithm>

namespace omnisbelli {

PlanetaryProjection ProjectionEngine::run_monte_carlo(
    const ArmyRoster& defenders_template,
    const ArmyRoster& attackers_template,
    const SectorGraph& sector_graph_template,
    const LogisticsSystem& logistics_template,
    int iterations
) {
    if (iterations <= 0) iterations = 10;

    int wins = 0;
    int stalemates = 0;
    int defeats = 0;

    double total_sim_time = 0.0;
    double total_human_losses = 0.0;
    double total_swarm_losses = 0.0;
    double total_ammo_depletion_time = 0.0;

    std::mt19937 rng(1337);
    std::uniform_real_distribution<double> variance_dist(0.85, 1.15);

    for (int iter = 0; iter < iterations; iter++) {
        ArmyRoster def = defenders_template;
        ArmyRoster att = attackers_template;
        SectorGraph sec = sector_graph_template;
        LogisticsSystem log = logistics_template;
        SynapseNetwork syn;

        double sim_time = 0.0;
        double dt = 60.0; // 60-second operational differential timestep
        double ammo_depleted_at = -1.0;

        CombatStepTelemetry tel;

        // Run simulation until resolution or timeout (up to 30 Terran days of planetary siege)
        const double max_combat_seconds = 30.0 * 86400.0;

        std::uniform_real_distribution<double> decap_chance(0.0, 1.0);
        bool decapitation_attempted = false;
        double next_doctrine_check = 3600.0 * (12.0 + decap_chance(rng) * 24.0);

        while (sim_time < max_combat_seconds) {
            if (def.is_defeated()) {
                defeats++;
                break;
            }
            if (att.is_defeated()) {
                wins++;
                break;
            }

            Sector& cur_sector = sec.get_active_sector();
            if (cur_sector.is_fallen()) {
                if (!sec.advance_active_sector()) {
                    // Citadel breached!
                    defeats++;
                    break;
                }
                continue;
            }

            // Check ammo depletion
            if (log.artillery_shells <= 1.0 && ammo_depleted_at < 0.0) {
                ammo_depleted_at = sim_time / 3600.0;
            }

            // High Command Tactical Strikes in Monte Carlo:
            // When battle reaches Sector 3 or 4, simulate probability of surgical Astartes drop-pod spearhead
            if (!decapitation_attempted && sim_time >= next_doctrine_check && sec.active_sector_index >= 2) {
                decapitation_attempted = true;
                if (decap_chance(rng) < 0.40) {
                    // Successful decapitation strike against Synapse nodes / commanders!
                    for (auto& u : att.units) {
                        if (u.is_alive() && (u.is_synapse || u.target_priority == TargetPriority::PRIORITY_SYNAPSE)) {
                            u.apply_direct_casualties(u.count * 0.45 + 500.0);
                        }
                    }
                    syn.update(att, 0.1);
                }
            }

            // Occasional Promethium Moat defense when breach is high (>60%)
            if (cur_sector.breach_percentage > 60.0 && log.can_afford_promethium(15000.0)) {
                log.consume_promethium(15000.0);
                double restored = cur_sector.max_integrity * 0.12;
                cur_sector.current_integrity = std::min(cur_sector.max_integrity, cur_sector.current_integrity + restored);
                cur_sector.breach_percentage = (1.0 - (cur_sector.current_integrity / cur_sector.max_integrity)) * 100.0;
            }

            // Slight random variance
            double var = variance_dist(rng);
            double effective_dt = dt * var;

            LanchesterCombat::compute_combat_tick(def, att, cur_sector, log, syn, effective_dt, tel);
            sim_time += effective_dt;

            // Early exit if attrition rate is negligible (stalemate)
            if (tel.defender_dps_effective < 1.0 && tel.attacker_dps_effective < 1.0) {
                stalemates++;
                break;
            }
        }

        if (sim_time >= max_combat_seconds) {
            stalemates++;
        }

        total_sim_time += sim_time;
        total_human_losses += def.get_total_casualties();
        total_swarm_losses += att.get_total_casualties();
        if (ammo_depleted_at >= 0.0) {
            total_ammo_depletion_time += ammo_depleted_at;
        } else {
            total_ammo_depletion_time += (sim_time / 3600.0);
        }
    }

    PlanetaryProjection result;
    result.iterations_run = iterations;
    result.win_percentage = (double(wins) / double(iterations)) * 100.0;
    result.stalemate_percentage = (double(stalemates) / double(iterations)) * 100.0;
    result.defeat_percentage = (double(defeats) / double(iterations)) * 100.0;
    result.expected_days_survived = (total_sim_time / double(iterations)) / 86400.0;
    result.projected_human_casualties = total_human_losses / double(iterations);
    result.projected_swarm_casualties = total_swarm_losses / double(iterations);
    result.projected_ammo_depletion_hour = total_ammo_depletion_time / double(iterations);

    if (result.win_percentage >= 60.0) {
        result.strategic_assessment = "FAVORABLE: DEFENSIVE FIREPOWER & CHOKEPOINTS INFLICT CATASTROPHIC ATTRITION.";
    } else if (result.win_percentage >= 25.0) {
        result.strategic_assessment = "CONTESTED: VICTORY REQUIRES RAPID SYNAPSE DECAPITATION & CONTROLLED SECTOR RETREATS.";
    } else if (result.stalemate_percentage >= 50.0) {
        result.strategic_assessment = "ATTRITION STALEMATE: FRONT PERMANENTLY ENTRENCHED. MUNITIONS DEPLETION CRITICAL.";
    } else {
        result.strategic_assessment = "DIRE: SWARM MASS OVERWHELMS FORTRESS ENVELOPE. EXTERMINATUS PREPARATION ADVISED.";
    }

    return result;
}

} // namespace omnisbelli

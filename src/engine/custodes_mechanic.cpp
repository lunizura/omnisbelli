#include "engine/custodes_mechanic.hpp"
#include "engine/army_roster.hpp"
#include "engine/sector.hpp"
#include <algorithm>

namespace omnisbelli {

CustodesMechanic::CustodesMechanic()
    : active_kata(KataStance::DACATARAI),
      stance_timer(0.0),
      auric_aegis_damage_prevented(0.0),
      rendax_armor_shred(0.25) {}

std::string CustodesMechanic::get_status_description() const {
    switch (active_kata) {
    case KataStance::KAPTARIS:
        return "MARTIAL KA'TAH: KAPTARIS [-30% INCOMING DAMAGE // PARRY BLADES]";
    case KataStance::DACATARAI:
        return "MARTIAL KA'TAH: DACATARAI [+40% SWARM CLEAVING DPS]";
    case KataStance::RENDAX:
        return "MARTIAL KA'TAH: RENDAX [+25% AP // TITAN & MONSTER HUNTER]";
    }
    return "MARTIAL KA'TAH ACTIVE";
}

std::string CustodesMechanic::get_telemetry_tag() const {
    switch (active_kata) {
    case KataStance::KAPTARIS: return "KA'TAH: KAPT";
    case KataStance::DACATARAI: return "KA'TAH: DACA";
    case KataStance::RENDAX: return "KA'TAH: REND";
    }
    return "KA'TAH";
}

double CustodesMechanic::get_telemetry_ratio() const {
    return 1.0; // Incorruptible perfection
}

void CustodesMechanic::pre_combat_tick(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    Sector& active_sector,
    double dt
) {
    (void)enemy;
    (void)active_sector;
    if (friendly.is_defeated() || dt <= 0.0) return;

    stance_timer += dt;
    // Automatically cycle stance every 35 seconds to adapt to battle rhythm
    if (stance_timer >= 35.0) {
        cycle_kata_stance();
        stance_timer = 0.0;
    }
}

void CustodesMechanic::modify_outgoing_damage(
    const ArmyRoster& friendly,
    DamagePacket& out_damage,
    double dt
) {
    (void)friendly;
    (void)dt;

    if (active_kata == KataStance::DACATARAI) {
        // Sweeping strikes cleaving through hordes
        out_damage.raw_damage *= 1.40;
    } else if (active_kata == KataStance::RENDAX) {
        // Monster-slayer stance with devastating armor pierce
        out_damage.raw_damage *= 1.25;
        out_damage.armor_penetration = std::min(0.90, out_damage.armor_penetration + rendax_armor_shred);
    }
}

double CustodesMechanic::apply_incoming_mitigation(
    ArmyRoster& friendly,
    double incoming_raw_damage,
    double dt
) {
    (void)friendly;
    (void)dt;
    if (incoming_raw_damage <= 0.0) return incoming_raw_damage;

    // 1. Auric Armor 2+ / 4++ Aegis of the Emperor: 35% flat mitigation
    double mitigated = incoming_raw_damage * 0.65;
    auric_aegis_damage_prevented += (incoming_raw_damage * 0.35);

    // 2. Kaptaris defensive parry posture absorbs an additional 25% of remaining damage
    if (active_kata == KataStance::KAPTARIS) {
        double kaptaris_prevented = mitigated * 0.25;
        mitigated *= 0.75;
        auric_aegis_damage_prevented += kaptaris_prevented;
    }

    return mitigated;
}

void CustodesMechanic::on_casualty_event(
    ArmyRoster& friendly,
    ArmyRoster& enemy,
    const CasualtyEvent& event
) {
    (void)friendly;
    (void)enemy;
    (void)event;
    // Custodes are emotionally immutable; no rout or panic mechanics apply
}

void CustodesMechanic::cycle_kata_stance() {
    switch (active_kata) {
    case KataStance::DACATARAI:
        active_kata = KataStance::RENDAX;
        break;
    case KataStance::RENDAX:
        active_kata = KataStance::KAPTARIS;
        break;
    case KataStance::KAPTARIS:
        active_kata = KataStance::DACATARAI;
        break;
    }
}

void CustodesMechanic::set_kata_stance(KataStance stance) {
    active_kata = stance;
    stance_timer = 0.0;
}

void CustodesMechanic::reset() {
    active_kata = KataStance::DACATARAI;
    stance_timer = 0.0;
    auric_aegis_damage_prevented = 0.0;
    rendax_armor_shred = 0.25;
}

} // namespace omnisbelli

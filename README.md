# OMNISBELLI // Planetary Siege Engine
> **A Macro-Scale Macroscopic Warfare Simulator & Tactical Command Cogitator**  
> *Forge World Tarsus Defense: 2,000,000 Imperial Defenders vs 100,000,000 Tyranid Swarm Biomass*

```
+==============================================================================+
| [COG] OMNISBELLI // ADEPTUS MECHANICUS COGITATOR // FORGE TARSUS DEFENSE     |
| TIME: DAY 14 // 08:34:12 // HIVE FLEET LEVIATHAN SWARM DETECTED              |
+==============================================================================+
| [SECTOR MAP: PLANETARY HIERARCHY]       | [TACTICAL CASUALTY TELEMETRY]      |
| [1] ORBITAL GRID   : [x] COMPROMISED    | IMPERIAL DEFENDERS:                |
| [2] ASH WASTES     : [x] OVERRUN        |   Active : 1,642,890 / 2,000,000   |
| [3] TRENCH ALPHA   : [!] UNDER SIEGE    |   Losses :   357,110 (17.9%)       |
|     Trench Breach  : [||||||....] 42%   | HIVE FLEET BIOMASS:                |
|     Chokepoint Ratio: 0.08x (LINEAR)    |   Swarm  : 78,412,040 / 100,000,000|
| [4] BASTION GATES  : [ ] READY          | ---------------------------------- |
| [5] HIVE SUB-LEVELS: [ ] READY          | SYNAPSE INTEGRITY: [||||||....] 58%|
| [6] SPIRE CITADEL  : [ ] READY          | MUNITIONS STATUS : [||||||||..] 72%|
+=========================================+====================================+
| [COMMAND ORDERS: ENGAGE ACTIVE DOCTRINES]                                    |
| [F1] PROMETHIUM MOAT  [F2] TITAN PLASMA  [F3] SPACE MARINE DROP-POD          |
| [F4] TACTICAL RETREAT [F5] ORBITAL LANCE [F8] EXTERMINATUS PROTOCOL          |
+==============================================================================+
```

---

## 1. Tactical Overview

**Omnisbelli** is a high-performance macro-scale macroscopic planetary combat simulator written in **Modern C++17/C++20** with **zero external library dependencies**.

Rather than simulating small skirmishes or grinding individual entities to a halt, the engine models macroscopic warfare involving **hundreds of millions of combatants** using:
- **Lanchester's Combat Laws** (Square Law in open ash wastes vs. Linear Law in narrow fortress gates and trench chokepoints).
- **Synaptic Network Degradation Graphs**: Severing Synapse nodes (Warriors, Zoanthropes) triggers **Instinctive Behavior (Feral Frenzy)**, halving swarm combat efficiency and causing mutual infighting.
- **Logistics & Munitions Depletion**: Ammunition stockpiles (Laspack MWh, Heavy Bolter rounds, Artillery shells, Promethium, Titan plasma reactor heat) actively burn down, degrading defensive firepower into desperate close-quarters fighting.
- **Multi-Tier Planetary Sector Defense**: 6 interconnected defensive echelons from the Orbital Defense Grid down to the Spire Citadel and Cogitator Core.
- **Monte Carlo Planetary Projections**: Real-time differential forecasting computing Win %, Stalemate %, and Defeat % probabilities.

---

## 2. Zero-Dependency Retro Pixel Architecture

- **Custom Software Virtual Framebuffer**: Native Windows Win32 32-bit DIBSection rendered at a retro virtual resolution of **640 × 360 pixels**.
- **Integer Nearest-Neighbor Hardware Upscaling**: Stretched via `StretchBlt(COLORONCOLOR)` to 1280×720 (2x), 1920×1080 (3x), or 4K/fullscreen with perfectly crisp square pixels and zero bilinear blurring.
- **Authentic Embedded Bitmap Font**: Embedded 8×8 and 16×16 font engines with custom tactical military glyphs:
  - `[COG]` Adeptus Mechanicus forge cog
  - `[AQUILA]` Imperial double-headed eagle
  - `[SKULL]` Imperial casualty marker
  - `[BIOHAZARD]` Tyranid swarm icon
  - `[CROSSHAIR]` Target priority lock
- **Tactical Bayer 4×4 Dithering**: Ordered matrix shading for radar elevation, sector defenses, and military progress bars.
- **Software CRT Scanline & Phosphor Shaders**:
  - `[P1]` **Mechanicus Green** (`#33FF33` on dark olive `#061206`)
  - `[P2]` **Amber Cathode** (`#FFB000` on warm sepia `#140A02`)
  - `[P3]` **Grimdark Slate** (`#99CCEE` ice blue on cold navy slate `#0A1016`)
  - `[P4]` **Cyber Cyan** (`#00F0FF` on deep void `#041018`)
  - `[C]` Toggle hardware scanline simulation.

---

## 3. Mathematical Combat Foundations

### 1. Lanchester's Attrition Laws (Open Field vs Chokepoint Physics)

In open planetary plains (Ash Wastes), combatants engage freely across the entire front (**Lanchester's Square Law**):
$$\frac{dH}{dt} = -k_T \cdot T(t)$$
$$\frac{dT}{dt} = -k_H \cdot H(t)$$

In narrow fortress breaches, trench funnels, and blast gates (Bastion Iron Gates), the attacking swarm's frontage is constrained by physical bottleneck width $W_{\text{choke}}$, forcing combat into **Lanchester's Linear Law**:
$$\text{Frontage Ratio } R_{\text{choke}} = \min\left(1.0, \frac{W_{\text{choke}}}{\sum \text{UnitCount}_i \times \text{Footprint}_i}\right)$$
$$\frac{dT_{\text{frontline}}}{dt} = -k_H \cdot H(t) \cdot \text{ChokepointMultiplier}$$

Defensive artillery and Titans inflict massive splash damage into the bottlenecked swarms waiting in queue.

### 2. Synaptic Disruption & Instinctive Behavior (Feral Frenzy)

Tyranid swarm combat lethality is sustained by the local presence of Synapse nodes:
$$\text{Synapse Integrity } \sigma = \frac{\sum \text{Living Synapse Creatures} \times \text{SynapseStrength}}{\text{Required Swarm Oversight}}$$
- **Coordinated State ($\sigma \ge 0.50$):** Swarm operates with 100% lethality.
- **Disrupted State ($0.25 \le \sigma < 0.50$):** Lethality drops by 30%, casualties increase by 30%.
- **Instinctive Behavior / Feral Frenzy ($\sigma < 0.25$):**
  - Swarm attack lethality drops by **70%** (mindless panic).
  - Swarm takes **double casualties (2.0x)** as organisms blindly rush into minefields and kill zones.
  - **Feral Infighting:** Non-synapse organisms panic and cannibalize each other (0.2% biomass loss per second).

---

## 4. Modular Faction Signature Mechanics

Omnisbelli features a pluggable `IFactionRuleModule` architecture simulating the signature mathematical combat laws of 9 major Warhammer 40,000 factions:

1. **Tyranid Hive Mind (`TyranidSynapseModule`)**:
   - Synapse network integrity $\sigma$ dynamically scales attack lethality and cohesion. Synapse disruption induces feral panic (+100% casualties, 0.2% cannibalistic infighting).
2. **Death Guard Vectorium (`DeathGuardMechanic`)**:
   - *Disgustingly Resilient*: 33.3% Feel No Pain (FNP) post-armor damage cancellation.
   - *Nurgle's Contagion Decay*: Radiating rot corrodes fortification cover and armor saves over time.
   - *Poxwalker Recursion*: Fallen frontline infantry rise as infectious plague thralls.
3. **Adepta Sororitas (`SororitasMechanic`)**:
   - *Acts of Faith & Miracle Dice*: Friendly martyrdom generates Miracle Points.
   - *Shield of Faith*: 4++ invulnerability field absorbing 50% of incoming damage.
   - *Holy Trinity*: Multi-melta, flamer, and bolter purity grant +35% damage and +0.15 AP.
4. **Adeptus Mechanicus (`MechanicusMechanic`)**:
   - *Canticles of the Omnissiah*: 4 binharic operational protocols cycling dynamically.
   - *Rad-Saturation*: High-yield radioactive bombardment deteriorating enemy bio-constructs.
5. **World Eaters (`WorldEatersMechanic`)**:
   - *Blood Tithe of Khorne*: 4 escalating tiers fueled by battlefield slaughter (+35% melee DPS, +0.15 AP).
   - *Spiteful Demise*: Fallen Berserkers explode into retaliatory counter-damage.
   - *Bloodthirster Apotheosis*: Greater Daemons summoned directly onto the frontline.
6. **Ork WAAAGH! (`OrkWaaaghMechanic`)**:
   - *WAAAGH! Momentum Curve*: Combat casualties and prolonged melee drive psychic momentum up to 100%, triggering a ferocious +40% melee DPS and charge AP surge.
   - *Dakkadakka Burst Hits*: Exploding hits provide +20% sustained burst damage and armor shred.
   - *Grot Shields*: Gretchin meatshields absorb 20% incoming anti-armor fire, shielding high-value Nobz and Gargants.
7. **Necron Dynasty (`NecronDynastyMechanic`)**:
   - *Reanimation Protocols*: 20% of fallen Necron warriors reconstitute each combat tick.
   - *Living Metal Hull Regeneration*: Monoliths and vehicles self-repair structural breaches and hull integrity.
   - *Phase-Out Protocol*: Emergency tomb world teleportation triggers when army falls below 25%.
8. **T'au Empire (`TauEmpireMechanic`)**:
   - *Markerlight Target Network*: Up to 10 markerlight tokens designating targets, negating enemy cover and granting +0.25 AP and seeker missile guidance (+30% DPS).
   - *Mont'ka vs. Kauyon*: Choose between Mont'ka (+35% early blitz DPS) and Kauyon (+40% sustained late-game fire).
   - *Shield Drone Interception*: Force field drones absorb 18% of incoming heavy anti-armor damage.
9. **Adeptus Custodes (`CustodesMechanic`)**:
   - *Martial Ka'tah Stances*: Cycles Dacatarai (anti-horde +40% DPS), Rendax (monster/titan slayer +25% DPS, +0.25 AP), and Kaptaris (parry defense).
   - *Auric Armor 2+/4++ Aegis*: 35% flat damage mitigation plus psychic immunity.

---

## 5. Modular Tactical Command Doctrines (`[F1]` - `[F8]`)

Doctrines dynamically configure based on the active defender faction:
- **Imperium**: Promethium Moat, Titan Plasma Volley, Astartes Drop-Pods, Tactical Retreat, Orbital Lance, Exterminatus.
- **Orks**: Da Big WAAAGH! Roar, Rokks From Da Sky, Green Tide Mob Call, Tactical Brawl Reposition, Dakka Burst Storm, Gargant Belly Gun Purge.
- **Necrons**: Reanimation Overcharge, Doomsday Arkanite Beam, Eternity Gate Ingress, Dimensional Dispersal, Solar Pulse Flare, Celestial Orrery Purge.
- **T'au Empire**: Markerlight Saturation, Seeker Missile Volley, Crisis Battlesuit Manta Drop, Kauyon Feigned Retreat, Tiger Shark Rail Barrage, Ion Core Overload.
- **Death Guard**: Miasma of Pestilence, Plagueburst Phosphex Barrage, Deathshroud Teleport, Slime-Trail Withdrawal, Rotting Corpse Cannonade, Life-Eater Virus.
- **World Eaters**: Blood for the Blood God, Lord of Skulls Cannonade, Red Butcher Assault Drop, Bloody Tactical Feint, Brass Meteor Strike, Skull Throne Apotheosis.
- **Adepta Sororitas**: Divine Aegis, Exorcist Pipe Organ Volley, Seraphim Descent, Sacred Martyrs Retreat, Retributor Holy Fire, Living Saint Divine Wrath.
- **Adeptus Mechanicus**: Binharic Canticle Override, Ordinatus Volcano Beam, Cybernetica Pulse, Calculated Logical Retreat, Rad-Bomb Saturation, Omnissiah Data-Purge.
- **Adeptus Custodes**: Martial Ka'tah Embolus, Telemon Storm Cannon, Allarus Ingress, Strategic Retrograde, Sagittarum Auric Volley, Talon Vault Forbidden Scourge.

---

## 6. External JSON Configuration Loader (`data/siege_config.json`)

Omnisbelli includes a zero-dependency JSON config loader (`ConfigLoader`) allowing users to adjust army rosters, statistics, and simulation parameters from disk without recompiling:
```json
{
  "defender_faction": "imperium",
  "attacker_faction": "tyranids",
  "time_multiplier": 5.0,
  "active_sector_index": 0,
  "units": [
    { "id": "cadian_infantry", "count": 1250000, "hp": 18.0, "armor": 0.20, "dps": 3.8 }
  ]
}
```

---

## 7. Controls

| Key | Action |
|:---|:---|
| `[1]` | Switch to **Tactical Command** Screen (Sector Hierarchy, Telemetry, Doctrines) |
| `[2]` | Switch to **2D Battlefield Radar** (Real-time animated tracers, explosions, clash line) |
| `[3]` | Switch to **Strategic Logistics** (Munitions stockpiles, burn rates, reactor heat) |
| `[4]` | Switch to **Unit Customizer** (Edit unit counts, plug custom armies) |
| `[5]` | Switch to **Monte Carlo Projections** (Win/Stalemate/Defeat probabilities) |
| `[F]` | Cycle Defender Faction (Imperium, Death Guard, Sororitas, Mechanicus, World Eaters, Custodes, Tau, Orks, Necrons) |
| `[G]` | Cycle Attacker Faction (Tyranids, Death Guard, World Eaters, Sororitas, Mechanicus, Orks, Necrons, Tau) |
| `[L]` | Cycle Planetary Campaign Scenario Presets (1 to 10) |
| `[SPACE]` | Pause / Resume Simulation |
| `[P]` | Cycle Phosphor Palette (Mechanicus Green, Amber, Slate, Cyber Cyan) |
| `[C]` | Toggle CRT Scanline Rasterizer Effect |
| `[` / `]` | Decrease / Increase Simulation Speed (0.5x, 1x, 5x, 20x, 100x) |
| `[M]` | Trigger 40-Iteration Monte Carlo Differential Calculation |
| `[R]` | Reset Simulation to Day 0 |
| `[F1 - F8]`| Engage Faction-Specific Tactical Command Doctrines |
| `[ESC]` | Exit Cogitator |

---

## 8. Compilation & Verification

### Build with Instant Batch Script (MSVC x64)
```cmd
build.bat
```
Produces `bin\omnisbelli.exe` and `bin\test_simulation.exe`.

### Run Automated Unit Test Suite
```cmd
test.bat
```
Verifies all 30 simulation test suites (Lanchester square vs linear laws, Synapse disruption, munitions depletion, sector fallback cascades, Monte Carlo forecasting, FNP & Contagion decay, Canticles, Miracle Dice, Blood Tithe, Ork WAAAGH! & Dakkadakka, Necron Reanimation & Living Metal, T'au Markerlights & Stances, Custodes Ka'tah, Faction Command Doctrines, JSON Config Customizer, and cross-faction doctrine coupling).

### Run Application
```cmd
run.bat
```

### Build with CMake
```cmd
cmake -B build -S .
cmake --build build --config Release
```

---

## 8. License
Omnisbelli is distributed under the MIT License.
Forged for planetary defense cogitators of the Imperium of Man.

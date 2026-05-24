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

## 4. Tactical Command Doctrines

- `[F1] IGNITE PROMETHIUM MOAT`: Floods sector trench with burning promethium fuel. Incinerates 2,500,000 swarm gaunts and sets back sector breach by 12%. Costs 15,000L fuel. (30s cooldown).
- `[F2] TITAN PLASMA VOLLEY`: Overcharges Warhound/Reaver Titan plasma destructors. Vaporizes 3,500 Carnifexes and 120 Bio-Titans. Generates +35% reactor heat. (45s cooldown).
- `[F3] SPACE MARINE DROP-POD SPEARHEAD`: Surgical Astartes decapitation strike targeting high-value Synapse nodes. Eliminates 15,000 Tyranid Warriors and 1,800 Zoanthropes, triggering instantaneous synaptic disruption. (60s cooldown).
- `[F4] ORDER TACTICAL FALLBACK`: Orders orderly withdrawal to the next defense line. Concedes the current sector but preserves 100% of surviving defenders from encirclement. (15s cooldown).
- `[F5] ORBITAL LANCE BATTERY`: Imperial Navy lance bombardment from high orbit. Incinerates 8,000,000 swarm organisms with collateral danger to friendly frontline. (90s cooldown).
- `[F8] EXTERMINATUS PROTOCOL`: Planetary cleansing via two-stage cyclonic torpedoes. Cleanses the entire planet in thermonuclear fire, denying biomass to the Hive Mind. *Unlockable once Bastion falls or Spire is under siege.*

---

## 5. User-Pluggable Roster & Scenario Presets

Press `[4]` to enter the **Interactive Roster Customizer**:
- Use `[UP/DOWN]` to select any military formation.
- Use `[LEFT/RIGHT]` to scale counts dynamically (`x0.5` or `x2.0`).
- Use `[TAB]` to switch between Defenders and Invaders.

### Built-in Scenario Presets (Keys `[1]` to `[5]` in Customizer):
1. **Fall of Tyran**: 1,978,020 Imperium vs 100,000,020 Hive Fleet Leviathan Swarm.
2. **Macragge Polar Fortress**: 4,000,000 Ultramar Defenders vs 250,000,000 Tyranids.
3. **Krieg 88th Siege Army**: 10,000,000 Death Korps vs 50,000,000 Tyranids.
4. **Third War for Armageddon**: 3,000,000 Imperium vs 90,000,000 Ork WAAAGH! (Boyz, Nobz, Battlewagons, Gargants).
5. **Tomb World Awakening**: 2,000,000 Imperium vs 35,000,000 Necron Dynasty (Gauss Warriors, Immortals, Monoliths).

---

## 6. Controls

| Key | Action |
|:---|:---|
| `[1]` | Switch to **Tactical Command** Screen (Sector Hierarchy, Telemetry, Doctrines) |
| `[2]` | Switch to **2D Battlefield Radar** (Real-time animated tracers, explosions, clash line) |
| `[3]` | Switch to **Strategic Logistics** (Munitions stockpiles, burn rates, reactor heat) |
| `[4]` | Switch to **Unit Customizer** (Edit unit counts, plug custom armies) |
| `[5]` | Switch to **Monte Carlo Projections** (Win/Stalemate/Defeat probabilities) |
| `[SPACE]` | Pause / Resume Simulation |
| `[P]` | Cycle Phosphor Palette (Mechanicus Green, Amber, Slate, Cyber Cyan) |
| `[C]` | Toggle CRT Scanline Rasterizer Effect |
| `[` / `]` | Decrease / Increase Simulation Speed (0.5x, 1x, 5x, 20x, 100x) |
| `[M]` | Trigger 40-Iteration Monte Carlo Differential Calculation |
| `[R]` | Reset Simulation to Day 0 |
| `[F1 - F8]`| Engage Tactical Command Doctrines |
| `[ESC]` | Exit Cogitator |

---

## 7. Compilation & Verification

### Build with Instant Batch Script (MSVC x64)
```cmd
build.bat
```
Produces `bin\omnisbelli.exe` and `bin\test_simulation.exe`.

### Run Automated Unit Test Suite
```cmd
test.bat
```
Verifies all 8 simulation test suites (Lanchester square vs linear laws, Synapse disruption, munitions depletion, sector fallback cascades, Monte Carlo forecasting, and modular rosters).

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

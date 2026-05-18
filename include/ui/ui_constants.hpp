#pragma once

namespace omnisbelli {

enum class ActiveScreenTab {
    TAB_TACTICAL = 0,   // Sector map, casualty matrix, command orders, dispatch ticker
    TAB_RADAR = 1,      // 2D real-time tactical radar visualizer with tracer fire & blast rings
    TAB_LOGISTICS = 2,  // Munitions stockpiles, supply burn rates, energy grid
    TAB_CUSTOMIZER = 3, // User-pluggable unit roster & scenario preset editor
    TAB_PROJECTION = 4  // Monte Carlo outcome forecasts & Lanchester attrition curves
};

} // namespace omnisbelli

#pragma once
#include "gfx/framebuffer.hpp"
#include <cstdint>
#include <string>

namespace omnisbelli {

enum class PhosphorPaletteType {
    MECHANICUS_GREEN,
    AMBER_CATHODE,
    GRIMDARK_SLATE,
    CYBER_CYAN
};

struct ThemeColors {
    uint32_t text_primary;
    uint32_t text_dim;
    uint32_t text_highlight;
    uint32_t bg_screen;
    uint32_t bg_panel;
    uint32_t border;
    uint32_t alert_amber;
    uint32_t alert_red;
    uint32_t bar_fill;
    uint32_t bar_empty;
    uint32_t friendly_unit;
    uint32_t hostile_unit;
    uint32_t synapse_glow;
};

class CRTEffects {
public:
    PhosphorPaletteType active_palette;
    bool scanlines_enabled;
    float scanline_dim; // 0.25f (25% darkening)

    CRTEffects();

    void cycle_palette();
    void set_palette(PhosphorPaletteType type);
    void toggle_scanlines();

    ThemeColors get_current_theme() const;
    std::string get_palette_name() const;

    void post_process(PixelFramebuffer& fb) const;
};

} // namespace omnisbelli

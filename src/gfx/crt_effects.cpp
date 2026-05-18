#include "gfx/crt_effects.hpp"
#include <algorithm>

namespace omnisbelli {

CRTEffects::CRTEffects()
    : active_palette(PhosphorPaletteType::MECHANICUS_GREEN),
      scanlines_enabled(true),
      scanline_dim(0.25f) {}

void CRTEffects::cycle_palette() {
    switch (active_palette) {
    case PhosphorPaletteType::MECHANICUS_GREEN:
        active_palette = PhosphorPaletteType::AMBER_CATHODE;
        break;
    case PhosphorPaletteType::AMBER_CATHODE:
        active_palette = PhosphorPaletteType::GRIMDARK_SLATE;
        break;
    case PhosphorPaletteType::GRIMDARK_SLATE:
        active_palette = PhosphorPaletteType::CYBER_CYAN;
        break;
    case PhosphorPaletteType::CYBER_CYAN:
    default:
        active_palette = PhosphorPaletteType::MECHANICUS_GREEN;
        break;
    }
}

void CRTEffects::set_palette(PhosphorPaletteType type) {
    active_palette = type;
}

void CRTEffects::toggle_scanlines() {
    scanlines_enabled = !scanlines_enabled;
}

std::string CRTEffects::get_palette_name() const {
    switch (active_palette) {
    case PhosphorPaletteType::MECHANICUS_GREEN:
        return "P1: MECHANICUS GREEN (FORGE TARSUS)";
    case PhosphorPaletteType::AMBER_CATHODE:
        return "P2: AMBER CATHODE (COMMAND RADAR)";
    case PhosphorPaletteType::GRIMDARK_SLATE:
        return "P3: GRIMDARK SLATE (NAVY TACTICAL)";
    case PhosphorPaletteType::CYBER_CYAN:
        return "P4: CYBER CYAN (VOX COGITATOR)";
    default:
        return "STANDARD";
    }
}

ThemeColors CRTEffects::get_current_theme() const {
    ThemeColors theme;
    switch (active_palette) {
    case PhosphorPaletteType::AMBER_CATHODE:
        theme.text_primary    = 0x00FFB000;
        theme.text_dim        = 0x00996600;
        theme.text_highlight  = 0x00FFF0A0;
        theme.bg_screen       = 0x00140A02;
        theme.bg_panel        = 0x00241206;
        theme.border          = 0x00AA7700;
        theme.alert_amber     = 0x00FFD700;
        theme.alert_red       = 0x00FF4411;
        theme.bar_fill        = 0x00FF9900;
        theme.bar_empty       = 0x00331805;
        theme.friendly_unit   = 0x00FFCC22;
        theme.hostile_unit    = 0x00FF3300;
        theme.synapse_glow    = 0x00FF0088;
        break;

    case PhosphorPaletteType::GRIMDARK_SLATE:
        theme.text_primary    = 0x0099CCEE;
        theme.text_dim        = 0x00446688;
        theme.text_highlight  = 0x00E0F0FF;
        theme.bg_screen       = 0x000A1016;
        theme.bg_panel        = 0x00141E28;
        theme.border          = 0x00447799;
        theme.alert_amber     = 0x00FFCC44;
        theme.alert_red       = 0x00EE2222;
        theme.bar_fill        = 0x005599CC;
        theme.bar_empty       = 0x00101820;
        theme.friendly_unit   = 0x0088CCFF;
        theme.hostile_unit    = 0x00FF3333;
        theme.synapse_glow    = 0x00CC33FF;
        break;

    case PhosphorPaletteType::CYBER_CYAN:
        theme.text_primary    = 0x0000F0FF;
        theme.text_dim        = 0x00007788;
        theme.text_highlight  = 0x00C0FFFF;
        theme.bg_screen       = 0x00041018;
        theme.bg_panel        = 0x000A1E2C;
        theme.border          = 0x0000AACC;
        theme.alert_amber     = 0x00FFB800;
        theme.alert_red       = 0x00FF0055;
        theme.bar_fill        = 0x0000E0FF;
        theme.bar_empty       = 0x00062030;
        theme.friendly_unit   = 0x0033FFFF;
        theme.hostile_unit    = 0x00FF0066;
        theme.synapse_glow    = 0x00FF55FF;
        break;

    case PhosphorPaletteType::MECHANICUS_GREEN:
    default:
        theme.text_primary    = 0x0033FF33;
        theme.text_dim        = 0x00167716;
        theme.text_highlight  = 0x0088FF88;
        theme.bg_screen       = 0x00061206;
        theme.bg_panel        = 0x000C220C;
        theme.border          = 0x001E881E;
        theme.alert_amber     = 0x00FFAA00;
        theme.alert_red       = 0x00FF2222;
        theme.bar_fill        = 0x0022DD22;
        theme.bar_empty       = 0x00081C08;
        theme.friendly_unit   = 0x0044FF44;
        theme.hostile_unit    = 0x00FF3322;
        theme.synapse_glow    = 0x00DD22FF;
        break;
    }
    return theme;
}

void CRTEffects::post_process(PixelFramebuffer& fb) const {
    if (!scanlines_enabled) return;

    // Fast integer scanline dimming on odd rows (dim by 25%: multiply by 3, divide by 4)
    for (int y = 1; y < fb.height; y += 2) {
        uint32_t* row = &fb.pixels[y * fb.width];
        for (int x = 0; x < fb.width; x++) {
            uint32_t c = row[x];
            uint32_t r = ((c >> 16) & 0xFF) * 3 / 4;
            uint32_t g = ((c >> 8) & 0xFF) * 3 / 4;
            uint32_t b = (c & 0xFF) * 3 / 4;
            row[x] = (r << 16) | (g << 8) | b;
        }
    }
}

} // namespace omnisbelli

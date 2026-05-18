#pragma once
#include "gfx/framebuffer.hpp"
#include <string>
#include <cstdint>

namespace omnisbelli {

enum class TacticalGlyph {
    SKULL,              // Imperial death/casualty symbol
    AQUILA,             // Double-headed eagle
    COG,                // Adeptus Mechanicus forge cog
    CROSSHAIR,          // Targeting reticle
    BIOHAZARD,          // Tyranid biomass swarm icon
    SHIELD,             // Void shield / armor
    WARNING_TRIANGLE,   // Alert indicator
    SWORD,              // Close combat
    CHOKEPONT_ICON      // Funnel bottleneck
};

class BitmapFont {
public:
    static void draw_char(
        PixelFramebuffer& fb,
        int x, int y,
        char c,
        uint32_t fg_color,
        uint32_t bg_color = 0x00000000,
        bool transparent = true,
        int scale = 1
    );

    static void draw_string(
        PixelFramebuffer& fb,
        int x, int y,
        const std::string& text,
        uint32_t fg_color,
        uint32_t bg_color = 0x00000000,
        bool transparent = true,
        int scale = 1
    );

    static void draw_glyph(
        PixelFramebuffer& fb,
        int x, int y,
        TacticalGlyph glyph,
        uint32_t fg_color,
        int scale = 1
    );

    static int get_string_width(const std::string& text, int scale = 1);
    static int get_char_height(int scale = 1);
};

} // namespace omnisbelli

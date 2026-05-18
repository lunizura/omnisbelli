#pragma once
#include "gfx/framebuffer.hpp"
#include <cstdint>
#include <vector>

namespace omnisbelli {

struct CombatParticle {
    float x, y;
    float vx, vy;
    float life;
    float max_life;
    uint32_t color;
    int size;
};

class Rasterizer {
public:
    static void draw_line(
        PixelFramebuffer& fb,
        int x0, int y0,
        int x1, int y1,
        uint32_t color
    );

    static void draw_hline(
        PixelFramebuffer& fb,
        int x, int y, int length,
        uint32_t color
    );

    static void draw_vline(
        PixelFramebuffer& fb,
        int x, int y, int length,
        uint32_t color
    );

    static void draw_rect(
        PixelFramebuffer& fb,
        int x, int y, int w, int h,
        uint32_t color
    );

    static void fill_rect(
        PixelFramebuffer& fb,
        int x, int y, int w, int h,
        uint32_t color
    );

    static void fill_dithered_rect(
        PixelFramebuffer& fb,
        int x, int y, int w, int h,
        uint32_t fg_color,
        uint32_t bg_color,
        int density_level // 0 (empty) to 16 (solid)
    );

    static void draw_frame(
        PixelFramebuffer& fb,
        int x, int y, int w, int h,
        uint32_t border_color,
        uint32_t bg_color = 0x00081008,
        bool has_header_accent = false
    );

    static void draw_progress_bar(
        PixelFramebuffer& fb,
        int x, int y, int w, int h,
        double ratio, // 0.0 to 1.0
        uint32_t fill_color,
        uint32_t empty_color,
        uint32_t border_color,
        bool segmented = true
    );

    static void draw_circle(
        PixelFramebuffer& fb,
        int xc, int yc, int r,
        uint32_t color
    );

    static void fill_circle(
        PixelFramebuffer& fb,
        int xc, int yc, int r,
        uint32_t color
    );
};

} // namespace omnisbelli

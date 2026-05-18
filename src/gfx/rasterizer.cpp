#include "gfx/rasterizer.hpp"
#include <algorithm>
#include <cmath>

namespace omnisbelli {

static const int BAYER_4X4[4][4] = {
    { 0,  8,  2, 10},
    {12,  4, 14,  6},
    { 3, 11,  1,  9},
    {15,  7, 13,  5}
};

void Rasterizer::draw_line(
    PixelFramebuffer& fb,
    int x0, int y0,
    int x1, int y1,
    uint32_t color
) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        fb.set_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Rasterizer::draw_hline(
    PixelFramebuffer& fb,
    int x, int y, int length,
    uint32_t color
) {
    if (y < 0 || y >= fb.height || length <= 0) return;
    int start_x = std::max(0, x);
    int end_x = std::min(fb.width - 1, x + length - 1);
    for (int px = start_x; px <= end_x; px++) {
        fb.set_pixel(px, y, color);
    }
}

void Rasterizer::draw_vline(
    PixelFramebuffer& fb,
    int x, int y, int length,
    uint32_t color
) {
    if (x < 0 || x >= fb.width || length <= 0) return;
    int start_y = std::max(0, y);
    int end_y = std::min(fb.height - 1, y + length - 1);
    for (int py = start_y; py <= end_y; py++) {
        fb.set_pixel(x, py, color);
    }
}

void Rasterizer::draw_rect(
    PixelFramebuffer& fb,
    int x, int y, int w, int h,
    uint32_t color
) {
    if (w <= 0 || h <= 0) return;
    draw_hline(fb, x, y, w, color);
    draw_hline(fb, x, y + h - 1, w, color);
    draw_vline(fb, x, y, h, color);
    draw_vline(fb, x + w - 1, y, h, color);
}

void Rasterizer::fill_rect(
    PixelFramebuffer& fb,
    int x, int y, int w, int h,
    uint32_t color
) {
    if (w <= 0 || h <= 0) return;
    int start_y = std::max(0, y);
    int end_y = std::min(fb.height - 1, y + h - 1);
    for (int py = start_y; py <= end_y; py++) {
        draw_hline(fb, x, py, w, color);
    }
}

void Rasterizer::fill_dithered_rect(
    PixelFramebuffer& fb,
    int x, int y, int w, int h,
    uint32_t fg_color,
    uint32_t bg_color,
    int density_level
) {
    if (w <= 0 || h <= 0) return;
    int density = std::clamp(density_level, 0, 16);

    int start_y = std::max(0, y);
    int end_y = std::min(fb.height - 1, y + h - 1);
    int start_x = std::max(0, x);
    int end_x = std::min(fb.width - 1, x + w - 1);

    for (int py = start_y; py <= end_y; py++) {
        int ry = py % 4;
        for (int px = start_x; px <= end_x; px++) {
            int rx = px % 4;
            bool hit = (density > BAYER_4X4[ry][rx]);
            fb.set_pixel(px, py, hit ? fg_color : bg_color);
        }
    }
}

void Rasterizer::draw_frame(
    PixelFramebuffer& fb,
    int x, int y, int w, int h,
    uint32_t border_color,
    uint32_t bg_color,
    bool has_header_accent
) {
    if (w <= 4 || h <= 4) return;

    // Fill background
    fill_rect(fb, x + 1, y + 1, w - 2, h - 2, bg_color);

    // Hard 1-pixel outer border
    draw_rect(fb, x, y, w, h, border_color);

    // Military corner notch accents (2x2 pixel blocks at corners)
    fb.set_pixel(x + 1, y + 1, border_color);
    fb.set_pixel(x + w - 2, y + 1, border_color);
    fb.set_pixel(x + 1, y + h - 2, border_color);
    fb.set_pixel(x + w - 2, y + h - 2, border_color);

    if (has_header_accent && h > 16) {
        // Horizontal divider under panel header bar
        draw_hline(fb, x, y + 14, w, border_color);
    }
}

void Rasterizer::draw_progress_bar(
    PixelFramebuffer& fb,
    int x, int y, int w, int h,
    double ratio,
    uint32_t fill_color,
    uint32_t empty_color,
    uint32_t border_color,
    bool segmented
) {
    if (w <= 4 || h <= 3) return;
    double clamped_ratio = std::clamp(ratio, 0.0, 1.0);

    // Border
    draw_rect(fb, x, y, w, h, border_color);

    int inner_w = w - 2;
    int inner_h = h - 2;
    int fill_w = static_cast<int>(std::round(inner_w * clamped_ratio));

    // Empty background
    fill_rect(fb, x + 1, y + 1, inner_w, inner_h, empty_color);

    if (fill_w > 0) {
        fill_rect(fb, x + 1, y + 1, fill_w, inner_h, fill_color);
    }

    if (segmented && inner_w > 20) {
        // Draw 1-pixel separator ticks every 10%
        for (int i = 1; i <= 9; i++) {
            int tick_x = x + 1 + (inner_w * i) / 10;
            draw_vline(fb, tick_x, y + 1, inner_h, border_color);
        }
    }
}

void Rasterizer::draw_circle(
    PixelFramebuffer& fb,
    int xc, int yc, int r,
    uint32_t color
) {
    if (r <= 0) {
        fb.set_pixel(xc, yc, color);
        return;
    }
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    auto plot_8 = [&](int px, int py) {
        fb.set_pixel(xc + px, yc + py, color);
        fb.set_pixel(xc - px, yc + py, color);
        fb.set_pixel(xc + px, yc - py, color);
        fb.set_pixel(xc - px, yc - py, color);
        fb.set_pixel(xc + py, yc + px, color);
        fb.set_pixel(xc - py, yc + px, color);
        fb.set_pixel(xc + py, yc - px, color);
        fb.set_pixel(xc - py, yc - px, color);
    };

    plot_8(x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        plot_8(x, y);
    }
}

void Rasterizer::fill_circle(
    PixelFramebuffer& fb,
    int xc, int yc, int r,
    uint32_t color
) {
    if (r <= 0) {
        fb.set_pixel(xc, yc, color);
        return;
    }
    for (int y = -r; y <= r; y++) {
        int half_w = static_cast<int>(std::sqrt(r * r - y * y));
        draw_hline(fb, xc - half_w, yc + y, half_w * 2 + 1, color);
    }
}

} // namespace omnisbelli

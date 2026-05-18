#include "gfx/framebuffer.hpp"
#include <cstring>
#include <algorithm>

namespace omnisbelli {

PixelFramebuffer::PixelFramebuffer(int w, int h)
    : width(w), height(h),
      hBitmap(NULL), bitmap_memory(nullptr), hdc_mem(NULL) {
    pixels.resize(width * height, 0x00081008);
}

PixelFramebuffer::~PixelFramebuffer() {
    if (hdc_mem) {
        DeleteDC(hdc_mem);
        hdc_mem = NULL;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }
}

void PixelFramebuffer::initialize_dib(HDC hdc_window) {
    if (hBitmap && hdc_mem) return;

    hdc_mem = CreateCompatibleDC(hdc_window);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hBitmap = CreateDIBSection(
        hdc_mem,
        &bmi,
        DIB_RGB_COLORS,
        &bitmap_memory,
        NULL,
        0
    );

    SelectObject(hdc_mem, hBitmap);
}

void PixelFramebuffer::clear(uint32_t color) {
    std::fill(pixels.begin(), pixels.end(), color);
}

void PixelFramebuffer::set_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    pixels[y * width + x] = color;
}

uint32_t PixelFramebuffer::get_pixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;
    return pixels[y * width + x];
}

int PixelFramebuffer::get_screen_scale(int client_width, int client_height) const {
    int scale_x = client_width / width;
    int scale_y = client_height / height;
    int scale = std::min(scale_x, scale_y);
    // Preliminary integer scaler implementation
    return std::max(1, scale);
}

void PixelFramebuffer::get_viewport_rect(int client_width, int client_height, int& vx, int& vy, int& vw, int& vh) const {
    int scale = get_screen_scale(client_width, client_height);
    vw = width * scale;
    vh = height * scale;
    vx = (client_width - vw) / 2;
    vy = (client_height - vh) / 2;
}

void PixelFramebuffer::screen_to_virtual(int client_x, int client_y, int client_width, int client_height, int& vx, int& vy) const {
    int v_ox, v_oy, v_w, v_h;
    get_viewport_rect(client_width, client_height, v_ox, v_oy, v_w, v_h);

    if (client_x < v_ox || client_x >= v_ox + v_w ||
        client_y < v_oy || client_y >= v_oy + v_h) {
        vx = -1;
        vy = -1;
        return;
    }

    int scale = get_screen_scale(client_width, client_height);
    vx = (client_x - v_ox) / scale;
    vy = (client_y - v_oy) / scale;
}

void PixelFramebuffer::present(HDC hdc_target, int client_width, int client_height) {
    if (!hBitmap || !hdc_mem || !bitmap_memory) {
        initialize_dib(hdc_target);
    }

    // Fast copy pixels to DIBSection memory
    std::memcpy(bitmap_memory, pixels.data(), width * height * sizeof(uint32_t));

    int vx, vy, vw, vh;
    get_viewport_rect(client_width, client_height, vx, vy, vw, vh);

    // Black letterbox bars outside virtual viewport
    if (vx > 0) {
        RECT rLeft = {0, 0, vx, client_height};
        RECT rRight = {vx + vw, 0, client_width, client_height};
        FillRect(hdc_target, &rLeft, (HBRUSH)GetStockObject(BLACK_BRUSH));
        FillRect(hdc_target, &rRight, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }
    if (vy > 0) {
        RECT rTop = {0, 0, client_width, vy};
        RECT rBottom = {0, vy + vh, client_width, client_height};
        FillRect(hdc_target, &rTop, (HBRUSH)GetStockObject(BLACK_BRUSH));
        FillRect(hdc_target, &rBottom, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }

    // Set nearest-neighbor scaling mode
    SetStretchBltMode(hdc_target, COLORONCOLOR);

    StretchBlt(
        hdc_target,
        vx, vy, vw, vh,
        hdc_mem,
        0, 0, width, height,
        SRCCOPY
    );
}

} // namespace omnisbelli

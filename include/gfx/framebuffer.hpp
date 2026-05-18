#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>

namespace omnisbelli {

constexpr int VIRTUAL_WIDTH = 640;
constexpr int VIRTUAL_HEIGHT = 360;

class PixelFramebuffer {
public:
    int width;
    int height;
    std::vector<uint32_t> pixels; // 0x00RRGGBB format

    HBITMAP hBitmap;
    void* bitmap_memory;
    HDC hdc_mem;

    PixelFramebuffer(int w = VIRTUAL_WIDTH, int h = VIRTUAL_HEIGHT);
    ~PixelFramebuffer();

    void clear(uint32_t color = 0x00081008);
    void set_pixel(int x, int y, uint32_t color);
    uint32_t get_pixel(int x, int y) const;

    void initialize_dib(HDC hdc_window);
    void present(HDC hdc_target, int client_width, int client_height);

    int get_screen_scale(int client_width, int client_height) const;
    void get_viewport_rect(int client_width, int client_height, int& vx, int& vy, int& vw, int& vh) const;
    void screen_to_virtual(int client_x, int client_y, int client_width, int client_height, int& vx, int& vy) const;
};

} // namespace omnisbelli

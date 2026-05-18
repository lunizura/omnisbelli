#include <windows.h>
#include <chrono>
#include "gfx/framebuffer.hpp"
#include "engine/simulation.hpp"
#include "ui/ui_manager.hpp"
#include "ui/input_handler.hpp"

using namespace omnisbelli;

static PixelFramebuffer g_framebuffer(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
static SimulationEngine g_simulation;
static UIManager g_ui;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        int client_w = rc.right - rc.left;
        int client_h = rc.bottom - rc.top;

        g_framebuffer.present(hdc, client_w, client_h);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1; // Prevent flicker

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }
        if (InputHandler::handle_key_down(wParam, g_simulation, g_ui)) {
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;

    case WM_LBUTTONDOWN: {
        RECT rc;
        GetClientRect(hWnd, &rc);
        int client_w = rc.right - rc.left;
        int client_h = rc.bottom - rc.top;

        int mouse_x = LOWORD(lParam);
        int mouse_y = HIWORD(lParam);

        int vx, vy;
        g_framebuffer.screen_to_virtual(mouse_x, mouse_y, client_w, client_h, vx, vy);
        if (InputHandler::handle_mouse_click(vx, vy, g_simulation, g_ui)) {
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"OmnisbelliSiegeWindowClass";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        return 1;
    }

    // Default window size: 1280 x 720 (Crisp 2x integer scale)
    int init_w = 1280;
    int init_h = 720;

    RECT wr = {0, 0, init_w, init_h};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    int window_w = wr.right - wr.left;
    int window_h = wr.bottom - wr.top;
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int start_x = (screen_w - window_w) / 2;
    int start_y = (screen_h - window_h) / 2;

    HWND hWnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"OMNISBELLI // Planetary Siege Engine [Adeptus Mechanicus Cogitator]",
        WS_OVERLAPPEDWINDOW,
        start_x, start_y, window_w, window_h,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) {
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // High resolution timer
    LARGE_INTEGER freq, prev_time, curr_time;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev_time);

    MSG msg = {};
    bool running = true;

    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running) break;

        // Compute delta time
        QueryPerformanceCounter(&curr_time);
        double dt = static_cast<double>(curr_time.QuadPart - prev_time.QuadPart) / static_cast<double>(freq.QuadPart);
        prev_time = curr_time;

        if (dt > 0.1) dt = 0.1; // Clamp spiral of death

        // Update simulation and visual particles
        g_simulation.update(dt);
        g_ui.update_visuals(dt, g_simulation);

        // Render to virtual pixel framebuffer
        g_ui.render(g_framebuffer, g_simulation);

        // Request repaint
        HDC hdc = GetDC(hWnd);
        RECT rc;
        GetClientRect(hWnd, &rc);
        int client_w = rc.right - rc.left;
        int client_h = rc.bottom - rc.top;

        g_framebuffer.present(hdc, client_w, client_h);
        ReleaseDC(hWnd, hdc);

        // Target ~60 FPS loop
        Sleep(16);
    }

    return static_cast<int>(msg.wParam);
}

// Fallback entry point for console builds
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOW);
}

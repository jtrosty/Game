// Not needed because i"m running with windows A funcitons
/*
#ifndef UNICODE
#define UNICODE
#endif 
*/

#include <windows.h>
#include "core.h"

struct Render_Buffer {
    int width;
    int height;
    u32* pixels;
    BITMAPINFO bitmap_info;
};

global_variable Render_Buffer render_buffer = {0};
global_variable char RUNNING = 1;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class.
    //const wchar_t CLASS_NAME[]  = L"Core Game Trost";
    
    WNDCLASSA window_class = {0};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc   = WindowProc;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = "Core_Window_Class";
    // window_class.lpfnWndProc = Win32MainWindowCallback;
    // WindowClass.hIcon;

    RegisterClassA(&window_class);

    //Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
    
    const int INITIAL_WINDOW_X = 200;
    const int INITIAL_WINDOW_Y = 200;

    HWND window = CreateWindowExA(
        0,                                  // Optional window styles.
        window_class.lpszClassName,         // Window class
        "Trost",                  // Window text
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,       // Position
        INITIAL_WINDOW_X, INITIAL_WINDOW_Y, // Size
        0,                               // Parent window    
        0,                               // Menu
        0,                          // Instance handle
        0 // Additional application data
        );

    if (window == NULL) {
        return 0;
    }

    HDC hdc = GetDC(window);


    int num_of_pixels = INITIAL_WINDOW_X * INITIAL_WINDOW_Y;

    render_buffer.height = INITIAL_WINDOW_X;
    render_buffer.width = INITIAL_WINDOW_Y;

    render_buffer.pixels = (u32*)VirtualAlloc(0, sizeof(32) * render_buffer.height * render_buffer.width,
                                        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    for (int i = 0; i < num_of_pixels; i++) {
        render_buffer.pixels[i] = 0xFFFF0000;
    }

    render_buffer.bitmap_info.bmiHeader.biBitCount = sizeof(render_buffer.bitmap_info.bmiHeader);
    render_buffer.bitmap_info.bmiHeader.biWidth = INITIAL_WINDOW_X;
    render_buffer.bitmap_info.bmiHeader.biHeight = INITIAL_WINDOW_Y;
    render_buffer.bitmap_info.bmiHeader.biPlanes = 1; // Must be set to 1
    render_buffer.bitmap_info.bmiHeader.biBitCount = 32; // 32 bits per pixel
    render_buffer.bitmap_info.bmiHeader.biCompression = BI_RGB; // Uncompressed
    render_buffer.bitmap_info.bmiHeader.biSizeImage = 0; // 0 for uncompressed
    render_buffer.bitmap_info.bmiHeader.biXPelsPerMeter = 0; // Not needed
    render_buffer.bitmap_info.bmiHeader.biYPelsPerMeter = 0; // Not needed
    render_buffer.bitmap_info.bmiHeader.biClrUsed = 0; // Not needed, this gets us the maximum amount of colors.
    render_buffer.bitmap_info.bmiHeader.biClrImportant = 0; // All colors are important.

    // TODO: ??
    //ShowWindow(window, nCmdShow);

    // Run the message loop.
    while (RUNNING) {
        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Render 
        StretchDIBits(
            hdc,                                // hdc,
            0, 0,                               // x and y dest,
            render_buffer.width, render_buffer.height, // Destwidth and height,
            0, 0,                               // x and y src,
            render_buffer.width, render_buffer.height, // src width and hegith,
            render_buffer.pixels,              // *lpbmi,
            &render_buffer.bitmap_info,         // bitmap infor,
            DIB_RGB_COLORS,                     // iUsage,
            SRCCOPY                             // rop
        );
    }

    return 0;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        RUNNING = 0;
        return 0;

    case WM_PAINT:
        {
            /*
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
            */
        }
        return 0;

    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
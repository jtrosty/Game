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

global_variable Render_Buffer render_buffer;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class.
    //const wchar_t CLASS_NAME[]  = L"Core Game Trost";
    char RUNNING = 1;
    
    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc   = WindowProc;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = "Core_Window_Class";
    // window_class.lpfnWndProc = Win32MainWindowCallback;
    // WindowClass.hIcon;

    RegisterClassA(&window_class);

    //Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
    
    const int INITIAL_WINDOW_X = 1280;
    const int INITIAL_WINDOW_Y = 720;

    HWND window = CreateWindowExA(
        0,                                  // Optional window styles.
        window_class.lpszClassName,         // Window class
        "Trost Core Game",                  // Window text
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,       // Position
        INITIAL_WINDOW_X, INITIAL_WINDOW_Y, // Size
        NULL,                               // Parent window    
        NULL,                               // Menu
        hInstance,                          // Instance handle
        NULL                                // Additional application data
        );

    if (window == NULL) {
        return 0;
    }

    HDC hdc = GetDC(window);

    // TODO: ??
    ShowWindow(window, nCmdShow);

    // Run the message loop.
    while (RUNNING) {
        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        StretchDIBits(
            hdc,                                // hdc,
            0, 0,                               // x and y dest,
            INITIAL_WINDOW_X, INITIAL_WINDOW_Y, // Destwidht and height,
            0, 0,                               // x and y src,
            INITIAL_WINDOW_X, INITIAL_WINDOW_Y, // src width and hegith,
            const BITMAPINFO                    // *lpbmi,
            DIB_RGB_COLORS,                                // iUsage,
            SRCCOPY                               // rop
        );
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
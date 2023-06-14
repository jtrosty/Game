// Not needed because i"m running with windows A funcitons
/*
#ifndef UNICODE
#define UNICODE
#endif 
*/

#include <windows.h>
#include <Xinput.h>
#include "core.h"

struct Render_Buffer {
    int width;
    int height;
    int num_of_pixels;
    u32* pixels;
    BITMAPINFO bitmap_info;
};

global_variable Render_Buffer render_buffer = {0};
global_variable char RUNNING = 1;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Xinput Get State
// This makes name represetn the funciton address on the right
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dw_user_index, XINPUT_STATE* p_state)
// This then 
typedef X_INPUT_GET_STATE(X_Input_Get_State);
X_INPUT_GET_STATE(xInputGetStateStub) {
    return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable X_Input_Get_State* x_input_get_state_ = xInputGetStateStub;
#define XInputGetState x_input_get_state_

// Xinput Set State
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dw_user_index, XINPUT_VIBRATION* p_vibration)
typedef X_INPUT_SET_STATE(X_Input_Set_State);
X_INPUT_SET_STATE(xInputSetStateStub) {
    return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable X_Input_Set_State* x_input_set_state_ = xInputSetStateStub;
#define XInputSetState x_input_set_state_

static void win32LoadXInput(void) {
    HMODULE x_input_library = LoadLibraryA("xinput1_4.dll");
    if (!x_input_library) {
        x_input_library = LoadLibraryA("xinput9_1_0.dll");
    }
    if (!x_input_library) {
        x_input_library = LoadLibraryA("xinput1_3.dll");
    }
    if (!x_input_library) {
        XInputGetState = (X_Input_Get_State*)GetProcAddress(x_input_library, "XInputGetState");
        if (!XInputGetState) {XInputGetState = xInputGetStateStub;}
        XInputSetState = (X_Input_Set_State*)GetProcAddress(x_input_library, "XInputSetState");
        if (!XInputSetState) {XInputSetState = xInputSetStateStub;}
    }
}

static void render_gradient(int x_offset) {
    for (int i = 0; i < render_buffer.num_of_pixels; i++) {
        render_buffer.pixels[i] = 0xFF0000ff + x_offset++;
        // 0xFF000000 black
        // 0xFFFF0000 orange maybe red
        // 0x00FF0000 red
        // 0x0000FF00 green
        // 0x000000FF blue

    }
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class.
    //const wchar_t CLASS_NAME[]  = L"Core Game Trost";
    // REMOVE
    int DEBUF_X_OFFSET = 0;
    
    WNDCLASSA window_class = {0};
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

    /*
    render_buffer.width = INITIAL_WINDOW_X;
    render_buffer.height = INITIAL_WINDOW_Y;
    render_buffer.num_of_pixels = INITIAL_WINDOW_X * INITIAL_WINDOW_Y;
    */

   win32LoadXInput();

    // Run the message loop.
    while (RUNNING) {

        DWORD dwResult;    
        for (DWORD i=0; i< XUSER_MAX_COUNT; i++ )
        {
            XINPUT_STATE state;
            ZeroMemory( &state, sizeof(XINPUT_STATE) );

            // Simply get the state of the controller from XInput.
            dwResult = XInputGetState( i, &state );

            if( dwResult == ERROR_SUCCESS )
            {
                // Controller is connected
                if (state.Gamepad.wButtons = 0x0100) {
                    // A is down
                    DEBUF_X_OFFSET += 20;
                }
            }
            else
            {
                // Controller is not connected
            }
        }
        MSG msg;
        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE) > 0)
        {
            switch (msg.message) {
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP: {

                    break;
                }
                default: {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
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

        render_gradient(DEBUF_X_OFFSET);
    }

    return 0;
}

static LRESULT CALLBACK WindowProc(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
        case WM_DESTROY: {
            PostQuitMessage(0);
            RUNNING = 0;
        } break;
        case WM_SIZE: {
            RECT rect;
            GetClientRect(window, &rect);
            render_buffer.width = rect.right - rect.left;
            render_buffer.height = rect.bottom - rect.top;
            render_buffer.num_of_pixels = render_buffer.width * render_buffer.height;

            if (render_buffer.pixels) {
                VirtualFree(render_buffer.pixels, 0, MEM_RELEASE);
            }
            render_buffer.pixels = (u32*)VirtualAlloc(0, sizeof(u32) * render_buffer.num_of_pixels, 
                                                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            render_buffer.bitmap_info.bmiHeader.biSize = sizeof(render_buffer.bitmap_info.bmiHeader);
            render_buffer.bitmap_info.bmiHeader.biWidth = render_buffer.width;
            render_buffer.bitmap_info.bmiHeader.biHeight = render_buffer.height;
            render_buffer.bitmap_info.bmiHeader.biPlanes = 1; // Must be set to 1
            render_buffer.bitmap_info.bmiHeader.biBitCount = 32; // 32 bits per pixel
            render_buffer.bitmap_info.bmiHeader.biCompression = BI_RGB; // Uncompressed
            render_buffer.bitmap_info.bmiHeader.biSizeImage = 0; // 0 for uncompressed
            render_buffer.bitmap_info.bmiHeader.biXPelsPerMeter = 0; // Not needed
            render_buffer.bitmap_info.bmiHeader.biYPelsPerMeter = 0; // Not needed
            render_buffer.bitmap_info.bmiHeader.biClrUsed = 0; // Not needed, this gets us the maximum amount of colors.
            render_buffer.bitmap_info.bmiHeader.biClrImportant = 0; // All colors are important.
        } break;
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
        default: {
            result = DefWindowProcA(window, uMsg, wParam, lParam);
        }
    }
    return result;
}
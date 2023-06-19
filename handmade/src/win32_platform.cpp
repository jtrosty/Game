// Not needed because i"m running with windows A funcitons
/*
#ifndef UNICODE
#define UNICODE
#endif 

/*
    Gamepad basics: done 15 june
    Keyboard: done 16 June
    Sound:
    frame rate
    saved last 20 secs
    tile map
*/

#include <windows.h>
#include <Xinput.h>
#include <dsound.h>
#include "core.h"
#include "win32_platform.h"

struct Win32_Offscreen_Buffer {
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO info;
    void* memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

struct Win32_Sound_Buffer {
    int samples_per_second;
    uint32 running_sample_index;
    int bytes_per_sample;
    DWORD secondary_buffer_size;
    DWORD safety_bytes;
    real32 t_sine;
    // TODO: Should running sample index be in bytes as well
    // TODO: Math gets simpler if we add a "bytes per second" field?
};

struct Win32_Window_Dimension{
    int width;
    int height;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct Win32_Replay_Buffer {
    HANDLE file_handle;
    HANDLE memory_map;
    char file_name[WIN32_STATE_FILE_NAME_COUNT];
    void* memory_block;
};

struct Win32_State {
    uint64 total_size;
    void* game_memory_block;
    Win32_Replay_Buffer replay_buffers[4];
    
    HANDLE recording_handle;
    int input_recording_index;

    HANDLE playback_handle;
    int input_playing_index;
    
    char EXE_file_name[WIN32_STATE_FILE_NAME_COUNT];
    char* one_past_last_EXE_filename_slash;
};

struct Win32_Render_Buffer {
    int width;
    int height;
    int num_of_pixels;
    u32* pixels;
    BITMAPINFO bitmap_info;
};


global_variable Win32_Render_Buffer render_buffer = {0};
global_variable char RUNNING = 1;
global_variable i64 global_perf_count_frequency;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void win32ProcessPendingMessage(Game_Controller_Input* keyboard_controller);
static void win32LoadXInput(void);
static void win32_process_x_input_button(DWORD x_input_state, Game_Button_State* old_state, 
                                         DWORD button_bit, Game_Button_State* new_state);
static real32 win32_process_x_input_stick(SHORT value, SHORT dead_zone_threshold);
static void win32_process_keyboard_message(Game_Button_State* new_state, bool32 is_down);
// DSound Shenanigans
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(_DirectSoundCreate);

// Xinput Get State /////////////////////////////////////////////////////////////
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
    if (x_input_library) {
        XInputGetState = (X_Input_Get_State*)GetProcAddress(x_input_library, "XInputGetState");
        if (!XInputGetState) {XInputGetState = xInputGetStateStub;}
        XInputSetState = (X_Input_Set_State*)GetProcAddress(x_input_library, "XInputSetState");
        if (!XInputSetState) {XInputSetState = xInputSetStateStub;}
    }
}

static void win32_process_x_input_button(DWORD x_input_state, Game_Button_State* old_state, 
                                         DWORD button_bit, Game_Button_State* new_state) {
    new_state->ended_down = ((x_input_state & button_bit) == button_bit);
    new_state->half_transition_count = (old_state->ended_down != new_state->ended_down) ? 1 : 0;
}

static real32 win32_process_x_input_stick(SHORT value, SHORT dead_zone_threshold) {
    real32 result = 0.0f;
    if (value < -dead_zone_threshold) {
        result = (real32)((value + dead_zone_threshold) / (32768.0f - dead_zone_threshold));
    }
    else if (value > dead_zone_threshold) {
        result = (real32)((value - dead_zone_threshold) / (32768.0f - dead_zone_threshold));
    }
    return result;
}

static void win32_process_keyboard_message(Game_Button_State* new_state, bool32 is_down){
    if (new_state->ended_down != is_down) {
        new_state->ended_down = is_down;
        ++new_state->half_transition_count;
    }
}
//******************************** END INPUT Section ************************************//

//******************************* Direct Sound Section *********************************//
static void win32_init_direct_sound(HWND window, i32 samples_per_second, i32 buffer_size) {
    HMODULE d_sound_library = LoadLibraryA("dsound.dll");
    if (d_sound_library) {

        _DirectSoundCreate* directSoundCreate = (_DirectSoundCreate*)GetProcAddress(d_sound_library, "DirectSoundCreate");

        LPDIRECTSOUND direct_sound;
        HRESULT debug_create_dsound = directSoundCreate(NULL, &direct_sound, NULL);
         if (debug_create_dsound == DS_OK) {
            WAVEFORMATEX pri_wave_format = {};
            pri_wave_format.wFormatTag = WAVE_FORMAT_PCM; 
            pri_wave_format.nChannels = 2;
            pri_wave_format.nSamplesPerSec = 48000;
            pri_wave_format.wBitsPerSample = 16;
            pri_wave_format.nBlockAlign = (pri_wave_format.wBitsPerSample * pri_wave_format.nChannels) / 8;
            pri_wave_format.nAvgBytesPerSec = pri_wave_format.nSamplesPerSec * pri_wave_format.nBlockAlign;
            pri_wave_format.cbSize = 0; // This is ignored when using WAVE_FORMAT_PCM

            DSBUFFERDESC pri_buffer_descriptipn = {};
            pri_buffer_descriptipn.dwSize = sizeof(pri_buffer_descriptipn);
            pri_buffer_descriptipn.dwFlags = DSBCAPS_PRIMARYBUFFER;
            pri_buffer_descriptipn.dwBufferBytes = 0; // Must be zero for primary buffer
            pri_buffer_descriptipn.dwReserved = 0; // MUst be zero
            pri_buffer_descriptipn.lpwfxFormat = &pri_wave_format;
            pri_buffer_descriptipn.guid3DAlgorithm = GUID_NULL;

            LPDIRECTSOUNDBUFFER primary_sound_buffer = {};
            
            if(SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY))){
                if (SUCCEEDED(direct_sound->CreateSoundBuffer(&pri_buffer_descriptipn, &primary_sound_buffer, NULL))) {
                    HRESULT error = primary_sound_buffer->SetFormat(&pri_wave_format);
                    if (SUCCEEDED(error)) {
                        OutputDebugStringA("Primary buffer formatw as set. Direct sound initialized.");
                    }
                    else {
                        // TODO: Failed format
                    }
                } 
                else {
                    // TODO: Failed creating sound buffer
                }
            }
            else {
                // TODO: unable to set coopoerative level in direct sound
            }
            // Set secondary buffer
            DSBUFFERDESC sec_buffer_descriptipn = {};
            sec_buffer_descriptipn.dwSize = sizeof(sec_buffer_descriptipn);
            sec_buffer_descriptipn.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
            sec_buffer_descriptipn.dwBufferBytes = buffer_size; // Must be zero for primary buffer
            sec_buffer_descriptipn.dwReserved = 0; // MUst be zero
            sec_buffer_descriptipn.lpwfxFormat = &pri_wave_format;
            sec_buffer_descriptipn.guid3DAlgorithm = GUID_NULL;

            LPDIRECTSOUNDBUFFER global_secondary_buffer = {};
            HRESULT error = direct_sound->CreateSoundBuffer(&sec_buffer_descriptipn, &global_secondary_buffer, 0);
            if (SUCCEEDED(error)) {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
         }
         else {
            // TODO: Logging
         }
    }
    else {
        // TODO: Logging
    }
}
//**************************** END Direct Sound Section ********************************//

//**************************** Start Timing ********************************//
inline LARGE_INTEGER win32_getWallClock(void) {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline real32 win32_getSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    real32 result ((real32)(end.QuadPart - start.QuadPart) / (real32)global_perf_count_frequency);
    return result;
}

//**************************** END Timing   ********************************//


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
    Win32_State win32_state = {};
    LARGE_INTEGER perf_count_frequency_result;
    QueryPerformanceCounter(&perf_count_frequency_result);
    global_perf_count_frequency = perf_count_frequency_result.QuadPart;



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

    // SETUP GAME INPUT
   win32LoadXInput();
   Game_Input input[2] = {};
   Game_Input* new_input = &input[0];
   Game_Input* old_input = &input[1];
   
   // Window monitor refresh
   int monitor_refresh_hz = 60;
   HDC refresh_dc = GetDC(window);
   int win32_refresh_rate = GetDeviceCaps(refresh_dc, VREFRESH);
   ReleaseDC(window, refresh_dc);
   if (win32_refresh_rate > 1) {
        monitor_refresh_hz = win32_refresh_rate;
   }
   real32 game_update_hz = (monitor_refresh_hz / 2.0f);
   real32 target_seconds_per_frame = 1.0f / (real32)game_update_hz;

    // Setup sound
    Win32_Sound_Output sound_output = {};
    sound_output.samples_per_second = 48000;
    sound_output.bytes_per_sample = sizeof(i16) * 2;
    sound_output.secondary_buffer_size = sound_output.samples_per_second * sound_output.bytes_per_sample;
    sound_output.safety_bytes = (int)(((real32)sound_output.samples_per_second *
                                       (real32)sound_output.bytes_per_sample / game_update_hz)
                                       / 3.0f);

    win32_init_direct_sound(window, 4800, 480000);


    // Run the message loop.
    while (RUNNING) {


        //******************** INPUT SECTION ***************************//
        Game_Controller_Input* new_keyboard = getController(new_input, 0);
        Game_Controller_Input* old_keyboard = getController(old_input, 0);
        *new_keyboard = {0};
        new_keyboard->is_connected = true;
        for (int button_index = 0; button_index < ArrayCount(new_keyboard->buttons); button_index++) {
            new_keyboard->buttons[button_index].ended_down = old_keyboard->buttons[button_index].ended_down;
        }
        win32ProcessPendingMessage(new_keyboard);
        OutputDebugStringA("Test");
        
        // MOUSE
        POINT mouse_pos;
        GetCursorPos(&mouse_pos);
        ScreenToClient(window, &mouse_pos);
        new_input->mouse_x = mouse_pos.x;
        new_input->mouse_y = mouse_pos.y;
        new_input->mouse_z = 0;
        /*
        win32_process_keyboard_message(&new_input->mouse_buttons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
        win32_process_keyboard_message(&new_input->mouse_buttons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
        win32_process_keyboard_message(&new_input->mouse_buttons[2], GetKeyState(VK_RBUTTON) & (1 << 15));
        win32_process_keyboard_message(&new_input->mouse_buttons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
        win32_process_keyboard_message(&new_input->mouse_buttons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
        */

        DWORD dwResult;    
        for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++ )
        {
            XINPUT_STATE x_input_controller_state;
            ZeroMemory( &x_input_controller_state, sizeof(XINPUT_STATE));

            // +1, because the input[0] is for the keybaord and 1-4 is for controllers. 
            Game_Controller_Input* new_controller = getController(new_input, controller_index + 1);
            Game_Controller_Input* old_controller = getController(old_input, controller_index + 1);

            // Simply get the state of the controller from XInput.
            dwResult = XInputGetState(controller_index, &x_input_controller_state );

            if(dwResult == ERROR_SUCCESS)
            {

                new_controller->is_connected = true;
                new_controller->is_analog = old_controller->is_analog;
                // Controller is connected

                // Sticks
                new_controller->left_stick_average_x = win32_process_x_input_stick(
                    x_input_controller_state.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
                    );
                new_controller->left_stick_average_y = win32_process_x_input_stick(
                    x_input_controller_state.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
                    );
                new_controller->right_stick_average_x = win32_process_x_input_stick(
                    x_input_controller_state.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
                    );
                new_controller->right_stick_average_y = win32_process_x_input_stick(
                    x_input_controller_state.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
                    );


                // Buttons
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->action_down, 
                                             XINPUT_GAMEPAD_A, &new_controller->action_down);
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->action_left, 
                                             XINPUT_GAMEPAD_B, &new_controller->action_left);
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->action_right, 
                                             XINPUT_GAMEPAD_X, &new_controller->action_right);
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->action_up, 
                                             XINPUT_GAMEPAD_Y, &new_controller->action_up);
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->left_shoulder, 
                                             XINPUT_GAMEPAD_LEFT_SHOULDER, &new_controller->left_shoulder);
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->right_shoulder, 
                                             XINPUT_GAMEPAD_RIGHT_SHOULDER, &new_controller->right_shoulder);
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->start, 
                                             XINPUT_GAMEPAD_START, &new_controller->start);
                win32_process_x_input_button(x_input_controller_state.Gamepad.wButtons, &old_controller->back, 
                                             XINPUT_GAMEPAD_BACK, &new_controller->back);
            }
            else
            {
                // Controller is not connected
                new_controller->is_connected = false;
            }

            // DEBUG Controlls
            if (new_controller->left_stick_average_x) {
                DEBUF_X_OFFSET += (100 * new_controller->left_stick_average_x);
            }
            if (new_controller->action_down.ended_down) {
                // A is down
                DEBUF_X_OFFSET += 100;
            }
            if (new_keyboard->move_up.ended_down || new_keyboard->move_up.half_transition_count ) {
                DEBUF_X_OFFSET += 100;
            }
            if (new_keyboard->move_down.ended_down || new_keyboard->move_up.half_transition_count) {
                DEBUF_X_OFFSET -= 100;
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

        // Game input flip new and old.
        Game_Input* temp = new_input;
        new_input = old_input;
        old_input = temp;

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

static void win32ProcessPendingMessage(Game_Controller_Input* keyboard_controller) {
    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch(msg.message)
        {
            case WM_QUIT:
            {
                RUNNING = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)msg.wParam;

                // NOTE(casey): Since we are comparing WasDown to IsDown,
                // we MUST use == and != to convert these bit tests to actual
                // 0 or 1 values.
                bool32 was_down = ((msg.lParam & (1 << 30)) != 0);
                bool32 is_down = ((msg.lParam & (1 << 31)) == 0);
                if(was_down != is_down)
                {
                    if(VKCode == 'W')
                    {
                        win32_process_keyboard_message(&keyboard_controller->move_up, is_down);
                    }
                    else if(VKCode == 'A')
                    {
                        win32_process_keyboard_message(&keyboard_controller->move_left, is_down);
                    }
                    else if(VKCode == 'S')
                    {
                        win32_process_keyboard_message(&keyboard_controller->move_down, is_down);
                    }
                    else if(VKCode == 'D')
                    {
                        win32_process_keyboard_message(&keyboard_controller->move_right, is_down);
                    }
                    else if(VKCode == 'Q')
                    {
                        win32_process_keyboard_message(&keyboard_controller->left_shoulder, is_down);
                    }
                    else if(VKCode == 'E')
                    {
                        win32_process_keyboard_message(&keyboard_controller->right_shoulder, is_down);
                    }
                    else if(VKCode == VK_UP)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_up, is_down);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_left, is_down);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_down, is_down);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        win32_process_keyboard_message(&keyboard_controller->action_right, is_down);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        win32_process_keyboard_message(&keyboard_controller->start, is_down);
                    }
                    else if(VKCode == VK_SPACE)
                    {
                        win32_process_keyboard_message(&keyboard_controller->back, is_down);
                    }
                    /*
#if HANDMADE_INTERNAL
                    else if(VKCode == 'P')
                    {
                        if(IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
                    else if(VKCode == 'L')
                    {
                        if(IsDown)
                        {
                            if(State->InputPlayingIndex == 0)
                            {
                                if(State->InputRecordingIndex == 0)
                                {
                                    Win32BeginRecordingInput(State, 1);
                                }
                                else
                                {
                                    Win32EndRecordingInput(State);
                                    Win32BeginInputPlayBack(State, 1);
                                }
                            }
                            else
                            {
                                Win32EndInputPlayBack(State);
                            }
                        }
                    }
#endif
*/
                }

                bool32 alt_key_was_down = (msg.lParam & (1 << 29));
                if((VKCode == VK_F4) && alt_key_was_down)
                {
                    RUNNING = false;
                }
            } break;

            default:
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            } break;
        }
    }
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
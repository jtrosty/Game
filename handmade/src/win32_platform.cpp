// Not needed because i"m running with windows A funcitons
/*
#ifndef UNICODE
#define UNICODE
#endif 

/*
    Gamepad basics: done 15 june
    Keyboard: done 16 June
    Sound: We have a sound buffer 25 June
    frame rate:  I kind of have frames, 26 june
    dll stuff: 26 june 
    saved last 20 secs: starte 27
    tile map: 
*/
#include "core.h"

#include <windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <stdio.h>

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
global_variable LPDIRECTSOUNDBUFFER global_secondary_buffer;
global_variable Win32_Offscreen_Buffer global_back_buffer;

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

//**************************** Game Load and Save Section ******************************//

inline FILETIME win32_getLastWriteTime(char* filename) {
    FILETIME last_write_time = {};
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }
    return last_write_time;
}

static Win32_Game_Code win32_loadGameCode(char* source_dll_name, char* temp_dll_name) {
    Win32_Game_Code result = {};
    //result.dll_last_write_time = win32
    result.dll_last_write_time = win32_getLastWriteTime(source_dll_name);

    CopyFile(source_dll_name, temp_dll_name, FALSE);

    result.game_code_dll = LoadLibraryA(temp_dll_name);

    if (result.game_code_dll) {
        result.update_and_render_fn = (game_update_and_render*)
            GetProcAddress(result.game_code_dll, "GameUpdateAndRender");
        result.get_sound_samples_fn = (game_get_sound_samples*)
            GetProcAddress(result.game_code_dll, "GameGetSoundSamples");

        result.is_valid = (result.update_and_render_fn && result.update_and_render_fn);
    }

    if (!result.is_valid) {
        result.update_and_render_fn = 0;
        result.get_sound_samples_fn = 0;
    }
    
    return result;
}

static void Win32_unloadGameCode(Win32_Game_Code* game_code) {
    if (game_code->game_code_dll) {
        FreeLibrary(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }
    game_code->is_valid = false;
    game_code->update_and_render_fn = 0;
    game_code->get_sound_samples_fn = 0;
}

static void catStrings(size_t source_a_count, char* source_a, 
                       size_t source_b_count, char* source_b,
                       size_t dest_count, char* dest) {
    for (int index = 0; index < source_a_count; ++index) {
        *dest++ = *source_a++;
    }
    for (int index = 0; index < source_b_count; ++index) {
        *dest++ = *source_b++;
    }
    *dest++ = 0;
}

static int stringLength(char* string) {
    int count = 0; 
    while (*string++) {
        ++count;
    }
    return count;
}

static void win32_buildEXEPathFileName(Win32_State* state, char* filename, int dest_count, char* dest) {
    catStrings(state->one_past_last_EXE_filename_slash - state->EXE_file_name, state->EXE_file_name, 
                stringLength(filename), filename, dest_count, dest);
}


static void win32_getEXEFileName(Win32_State* state) {
    DWORD size_of_filename = GetModuleFileNameA(0, state->EXE_file_name, sizeof(state->EXE_file_name));
    state->one_past_last_EXE_filename_slash = state->EXE_file_name;
    for (char* scan = state->EXE_file_name; *scan; ++scan) {
        if (*scan == '\\') {
            state->one_past_last_EXE_filename_slash = scan + 1;
        }
    }
}


//********************************** END Section ***************************************//

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

static void win32_fillSoundBuffer(Win32_Sound_Output* sound_output, DWORD byte_to_lock, DWORD bytes_to_writes, Game_Sound_Output_Buffer* source_buffer) {
    VOID* region_1;
    DWORD region_1_size;
    VOID* region_2;
    DWORD region_2_size;
    if (SUCCEEDED(global_secondary_buffer->Lock(byte_to_lock, bytes_to_writes, 
                                                &region_1, &region_1_size, 
                                                &region_2, &region_2_size, 
                                                0))) {
        DWORD region_1_sample_count = region_1_size /sound_output->bytes_per_sample;
        i16* dest_sample = (i16*)region_1;
        i16* source_sample = (i16*)source_buffer->samples;
        for (DWORD sample_index = 0; sample_index < region_1_sample_count; ++sample_index) {
            *dest_sample++ = *source_sample++;
            *dest_sample++ = *source_sample++;

            ++sound_output->running_sample_index;
        }
        DWORD region_2_sample_count = region_2_size / sound_output->bytes_per_sample;
        dest_sample = (i16*)region_2;
        for (DWORD sample_index = 0; sample_index < region_2_sample_count; ++sample_index) {
            *dest_sample++ = *source_sample++;
            *dest_sample++ = *source_sample++;

            ++sound_output->running_sample_index;
        }
        global_secondary_buffer->Unlock(region_1, region_1_size, region_2, region_2_size);
    }
}

static void win32_clearBuffer(Win32_Sound_Output* sound_output) {
    VOID* region_1;
    DWORD region_1_size;
    VOID* region_2;
    DWORD region_2_size;
    if (SUCCEEDED(global_secondary_buffer->Lock(0, sound_output->secondary_buffer_size, 
                                                &region_1, &region_1_size, 
                                                &region_2, &region_2_size, 
                                                0))) {
        u8* dest_sample = (u8*)region_1;
        for (DWORD byte_index = 0; byte_index < region_1_size; ++byte_index) {
            *dest_sample = 0;
        }
        dest_sample = (u8*)region_2;
        for (DWORD byte_index = 0; byte_index < region_2_size; ++byte_index) {
            *dest_sample = 0;
        }
        global_secondary_buffer->Unlock(region_1, region_1_size, region_2, region_2_size);
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


//**************************** DEBUG FILE SECTION******************************//
DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platformFreeFileMemory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platformReadEntireFile) {
    Debug_Read_File_Result result = {};
    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if(GetFileSizeEx(file_handle, &file_size)) {
            u32 file_size_32 = safeTruncateU64(file_size.QuadPart);
            result.contents = VirtualAlloc(0, file_size_32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if(result.contents) {
                DWORD bytes_read;
                if(ReadFile(file_handle, result.contents, file_size_32, &bytes_read, 0) && file_size_32 == bytes_read) {
                    result.contents_size = file_size_32;
                }
                else {
                    debug_platformFreeFileMemory(Thread, result.contents);
                    result.contents = 0;
                }
            }
            else {
                // TODO: Logging
            }
        }
        else {
            // TODO: Logging
        }

        CloseHandle(file_handle);
    }
    else {
        // TODO: logging
    }
    return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platformWriteEntireFile) {
    bool32 result = false;
    HANDLE file_handle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if(WriteFile(file_handle, memory, memory_size, &bytes_written, 0)) {
            // NOTE: file read succesfully
            result = (bytes_written == memory_size);
        }
        else {
            // TODO: Logging
        }
        CloseHandle(file_handle);
    }
    else {
        // TODO: logging
    }
    return result;
}


//******************************** END DEBUG ************************************//

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

static void DEBUG_output_sound(Game_State* game_state, Game_Sound_Output_Buffer* sound_buffer, int tone_hz) {
    i16 tone_volume = 3000;
    int wave_period = sound_buffer->samples_per_second / tone_hz;

    i16* sample_out = sound_buffer->samples;
    for (int sample_index = 0; sample_index < sound_buffer->sample_count; ++sample_index) {
        real32 sine_value = sinf(game_state->t_sine);
        i16 sample_value = (i16)(sine_value * tone_volume);
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;

        game_state->t_sine += 2.0f * Pi32 * 1.0f / (real32)wave_period;
        if(game_state->t_sine > 2.0f * Pi32) {
            game_state->t_sine -= 2.0f * Pi32;
        }
    }
}
//*************************************************************************************

//************************* DISPLAY CODE ************************************************************
static void win32_resizeDIBSection(Win32_Offscreen_Buffer* buffer, int width, int height) {
    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }
    buffer->width = width;
    buffer->height = height;

    int bytes_per_pixel = 4;
    buffer->bytes_per_pixel = bytes_per_pixel;

    // bmi Header
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = buffer->height;
    buffer->info.bmiHeader.biPlanes = 1; // Must be set to 1
    buffer->info.bmiHeader.biBitCount = 32; // 32 bits per pixel
    buffer->info.bmiHeader.biCompression = BI_RGB; // Uncompressed
    buffer->info.bmiHeader.biSizeImage = 0; // 0 for uncompressed
    buffer->info.bmiHeader.biXPelsPerMeter = 0; // Not needed
    buffer->info.bmiHeader.biYPelsPerMeter = 0; // Not needed
    buffer->info.bmiHeader.biClrUsed = 0; // Not needed, this gets us the maximum amount of colors.
    buffer->info.bmiHeader.biClrImportant = 0; // All colors are important.

    int bitmap_memory_size = (buffer->width * buffer->height) * bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytes_per_pixel;
}

static void win32_displayBufferInWindow(Win32_Offscreen_Buffer* buffer, HDC device_context, int window_width, int window_height) {
    StretchDIBits(device_context, 
                  0, 0, buffer->width, buffer->height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory,
                  &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}

static Win32_Window_Dimension win32_getWindowDimension(HWND window) {
    Win32_Window_Dimension result;
    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;
    return result;
}
//*************************************************************************************

//************************* REPLAY CODE ***********************************************

static void win32_getInputFileLocation(Win32_State* state, bool32 input_stream, 
                                       int slot_index, int dest_count, char* dest) {
    char temp[64];
    wsprintf(temp, "loop_edit_&d_%d.hmi", slot_index, input_stream ? "input" : "state");
    win32_buildEXEPathFileName(state, temp, dest_count, dest);
}

static Win32_Replay_Buffer* win32_getReplayBuffer(Win32_State* state, int unsigned index) {
    Assert(index < ArrayCount(state->replay_buffers));
    Win32_Replay_Buffer* result = &state->replay_buffers[index];
    return result;
}

static void win32_beginRecordingInput(Win32_State* state, int input_recording_index) {
    Win32_Replay_Buffer* replay_buffer = win32_getReplayBuffer(state, input_recording_index);
    if (replay_buffer->memory_block) {
        state->input_recording_index = input_recording_index;
        char filename[WIN32_STATE_FILE_NAME_COUNT];
        win32_getInputFileLocation(state, true, input_recording_index, sizeof(filename), filename);
        state->recording_handle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
#if 0
        LARGE_INTEGER file_position;
        file_position.QuadPart = state->total_size;
        SetFilePointerEx(state->recording_handle, file_position, 0, FILE_BEGIN);
#endif
        CopyMemory(replay_buffer->memory_block, state->game_memory_block, state->total_size);
    }
}

static void win32_endRecordingInput(Win32_State* state) {
    CloseHandle(state->recording_handle);
    state->input_playing_index = 0;
}

static void win32_beginInputPlayBack(Win32_State* state, int input_playing_index) {
    Win32_Replay_Buffer* replay_buffer = win32_getReplayBuffer(state, input_playing_index);
    if(replay_buffer->memory_block) {
        state->input_playing_index = input_playing_index;

        char filename[WIN32_STATE_FILE_NAME_COUNT];
        win32_getInputFileLocation(state, true, input_playing_index, sizeof(filename), filename);
        state->playback_handle = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

#if 0
        LARGE_INTEGER file_position;
        file_position.QuadPart = state->total_size;
        SetFilePointerEx(state->recording_handle, file_position, 0, FILE_BEGIN);
#endif
        CopyMemory(state->game_memory_block, replay_buffer->memory_block, state->total_size);
    }
}

static void win32_endInputPlayBack(Win32_State* state) {
    CloseHandle(state->playback_handle);
    state->input_playing_index = 0;
}

static void win32_recordInput(Win32_State* state, Game_Input* new_input) {
    DWORD bytes_written;
    WriteFile(state->recording_handle, new_input, sizeof(*new_input), &bytes_written, 0);
}

static void win32_playBackInput(Win32_State* state, Game_Input* new_input) {
    DWORD bytes_read = 0;
    if(ReadFile(state->playback_handle, new_input, sizeof(*new_input), &bytes_read, 0)) {
        if (bytes_read == 0) {
            int playing_index = state->input_playing_index;
            win32_endInputPlayBack(state);
            win32_beginInputPlayBack(state, playing_index);
            ReadFile(state->playback_handle, new_input, sizeof(*new_input), &bytes_read, 0);
        }
    }
}

//************************* END REPLAY SECTION *****************************************
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class.
    //const wchar_t CLASS_NAME[]  = L"Core Game Trost";
    // REMOVE
    int DEBUF_X_OFFSET = 0;
    
    WNDCLASSA window_class = {0};

    win32_resizeDIBSection(&global_back_buffer, 1280, 720);

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

   
   // Window monitor refresh
    Win32_State win32_state = {};
    LARGE_INTEGER perf_count_frequency_result;
    QueryPerformanceCounter(&perf_count_frequency_result);
    global_perf_count_frequency = perf_count_frequency_result.QuadPart;

    win32_getEXEFileName(&win32_state);

    // Game code loading
    char source_game_code_dll_full_path[WIN32_STATE_FILE_NAME_COUNT];
    win32_buildEXEPathFileName(&win32_state, "core.dll", sizeof(source_game_code_dll_full_path), source_game_code_dll_full_path);

    char temp_game_code_dll_full_path[WIN32_STATE_FILE_NAME_COUNT];
    win32_buildEXEPathFileName(&win32_state, "core_temp.dll", sizeof(temp_game_code_dll_full_path), temp_game_code_dll_full_path);

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

    win32_init_direct_sound(window, 4800, sound_output.secondary_buffer_size);
    win32_clearBuffer(&sound_output);
    global_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
    i16* sound_samples = (i16*)VirtualAlloc(0, sound_output.secondary_buffer_size, 
                                            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL
    LPVOID base_address = 0;
    //LPVOID base_address = (LPVOID)Terabytes(2);
#else
    LPVOID base_address = 0;
#endif

    // Game memory loading
    Game_Memory game_memory = {};
    game_memory.permanent_storage_size = Megabytes(64);
    game_memory.transient_storage_size = Gigabytes(1);
    game_memory.DEBUG_platformFreeFileMemory = debug_platformFreeFileMemory;
    game_memory.DEBUG_platformReadEntireFile = debug_platformReadEntireFile;
    game_memory.DEBUG_platformWriteEntireFile = debug_platformWriteEntireFile;

    win32_state.total_size = game_memory.permanent_storage_size + game_memory.transient_storage_size;
    win32_state.game_memory_block = VirtualAlloc(base_address, (size_t)win32_state.total_size, 
                                                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    game_memory.permanent_storage = win32_state.game_memory_block;
    game_memory.transient_storage = ((u8*)game_memory.permanent_storage + game_memory.permanent_storage_size);

    for (int replay_index = 0; replay_index < ArrayCount(win32_state.replay_buffers); ++replay_index) {

    }

    if (sound_samples && game_memory.permanent_storage && game_memory.transient_storage) {

        // Setup sound stuff 
        DWORD audio_latency_bytes = 0;
        real32 audio_latency_seconds = 0;
        bool32 sound_is_valid = false;


            // SETUP GAME INPUT
        win32LoadXInput();
        Game_Input input[2] = {};
        Game_Input* new_input = &input[0];
        Game_Input* old_input = &input[1];

        LARGE_INTEGER last_counter = win32_getWallClock();
        LARGE_INTEGER flip_wall_clock = win32_getWallClock();
        uint64 last_cycle_count = __rdtsc();

        // DEBUG
        Game_State* game_state = (Game_State*)game_memory.permanent_storage;
        Win32_Game_Code game = win32_loadGameCode(source_game_code_dll_full_path, temp_game_code_dll_full_path);
        u32 load_counter = 0;

        // Run the message loop.
        while (RUNNING) {

            FILETIME new_dll_write_time = win32_getLastWriteTime(source_game_code_dll_full_path);
            if(CompareFileTime(&new_dll_write_time, &game.dll_last_write_time) != 0) {
                Win32_unloadGameCode(&game);
                game = win32_loadGameCode(source_game_code_dll_full_path, temp_game_code_dll_full_path);
                load_counter = 0;
            }

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

    /*
                // DEBUG Controlls
                if (new_controller->left_stick_average_x) {
                    DEBUF_X_OFFSET += (100.0 * new_controller->left_stick_average_x);
                }
                if (new_controller->action_down.ended_down) {
                    // A is down
                    DEBUF_X_OFFSET += 100.0;
                }
                if (new_keyboard->move_up.ended_down || new_keyboard->move_up.half_transition_count ) {
                    DEBUF_X_OFFSET += 100.0;
                }
                if (new_keyboard->move_down.ended_down || new_keyboard->move_up.half_transition_count) {
                    DEBUF_X_OFFSET -= 100.0;
                }
                */
            }

            Thread_Context thread = {};

            Game_Offscreen_Buffer buffer = {};
            buffer.memory = global_back_buffer.memory;
            buffer.width = global_back_buffer.width;
            buffer.height = global_back_buffer.height;
            buffer.pitch = global_back_buffer.pitch;
            buffer.bytes_per_pixel = global_back_buffer.bytes_per_pixel;

            if(game.update_and_render_fn) {
                game.update_and_render_fn(&thread, &game_memory, new_input, &buffer);
            }


            // Sound
            LARGE_INTEGER audio_wall_clock = win32_getWallClock();
            real32 from_begin_to_audio_seconds = win32_getSecondsElapsed(flip_wall_clock, audio_wall_clock);
            
            DWORD play_cursor;
            DWORD write_cursor;
            if (global_secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor) == DS_OK) {
                                /* NOTE(casey):

                                Here is how sound output computation works.

                                We define a safety value that is the number
                                of samples we think our game update loop
                                may vary by (let's say up to 2ms)
                        
                                When we wake up to write audio, we will look
                                and see what the play cursor position is and we
                                will forecast ahead where we think the play
                                cursor will be on the next frame boundary.

                                We will then look to see if the write cursor is
                                before that by at least our safety value.  If
                                it is, the target fill position is that frame
                                boundary plus one frame.  This gives us perfect
                                audio sync in the case of a card that has low
                                enough latency.

                                If the write cursor is _after_ that safety
                                margin, then we assume we can never sync the
                                audio perfectly, so we will write one frame's
                                worth of audio plus the safety margin's worth
                                of guard samples.
                                */
                if (!sound_is_valid) {
                    sound_output.running_sample_index = write_cursor / sound_output.bytes_per_sample;
                    sound_is_valid = true;
                }
                DWORD byte_to_lock = ((sound_output.running_sample_index * sound_output.bytes_per_sample) % 
                                        sound_output.secondary_buffer_size);
                DWORD expected_sound_bytes_per_frame = (int)((real32)sound_output.samples_per_second * sound_output.bytes_per_sample) 
                                                        / game_update_hz;
                real32 seconds_left_until_flip = (target_seconds_per_frame - from_begin_to_audio_seconds);
                DWORD expected_bytes_until_flip = (DWORD)((seconds_left_until_flip / target_seconds_per_frame) 
                                                * (real32)expected_sound_bytes_per_frame);

                DWORD expected_frame_boundry_byte = play_cursor + expected_bytes_until_flip;

                DWORD safe_write_cursor = write_cursor;
                if(safe_write_cursor < play_cursor) {
                    safe_write_cursor += sound_output.secondary_buffer_size;
                }
                Assert(safe_write_cursor >= play_cursor);
                safe_write_cursor += sound_output.safety_bytes;
                
                bool32 audio_card_is_low_latency = (safe_write_cursor < expected_frame_boundry_byte);

                DWORD target_cursor = 0;
                if (audio_card_is_low_latency) {
                    target_cursor = (expected_frame_boundry_byte + expected_sound_bytes_per_frame);
                }
                else {
                    target_cursor = (write_cursor + expected_sound_bytes_per_frame + sound_output.safety_bytes);
                }
                target_cursor = (target_cursor % sound_output.secondary_buffer_size);

                DWORD bytes_to_write = 0;
                if (byte_to_lock > target_cursor) {
                    bytes_to_write = (sound_output.secondary_buffer_size - byte_to_lock);
                    bytes_to_write += target_cursor;
                }
                else {
                    bytes_to_write = target_cursor - byte_to_lock;
                }
                Game_Sound_Output_Buffer sound_buffer = {};
                sound_buffer.samples_per_second = sound_output.samples_per_second;
                sound_buffer.sample_count = bytes_to_write / sound_output.bytes_per_sample;
                sound_buffer.samples = sound_samples;
                //game_state->tone_hz = 512;

                //DEBUG_output_sound(game_state, &sound_buffer, game_state->tone_hz);
                // READY FOR DLL
                if (game.get_sound_samples_fn) {
                    game.get_sound_samples_fn(&thread, &game_memory, &sound_buffer);
                }
                win32_fillSoundBuffer(&sound_output, byte_to_lock, bytes_to_write, &sound_buffer);
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

            Win32_Window_Dimension dimension = win32_getWindowDimension(window);
            HDC device_context = GetDC(window);
            win32_displayBufferInWindow(&global_back_buffer, device_context,
                                        dimension.width, dimension.height);
            ReleaseDC(window, device_context);
            //render_gradient(DEBUF_X_OFFSET);

            LARGE_INTEGER work_counter = win32_getWallClock();
            real32 work_seconds_elapsed = win32_getSecondsElapsed(last_counter, work_counter);


            LARGE_INTEGER end_counter = win32_getWallClock();
            real32 ms_per_frame = 1000.0f * win32_getSecondsElapsed(last_counter, end_counter);
            last_counter = end_counter;

            uint64 end_cycle_count = __rdtsc();
            uint64 cycles_elapsed = end_cycle_count - last_cycle_count;
            last_cycle_count = end_cycle_count;
            
            //real64 fps = perf_count_frequency_result / (end_counter.QuadPart - last_counter.QuadPart);
            real64 fps =0.0;
            real64 mega_cycles_per_frame = ((real64)cycles_elapsed / (1000.0f * 1000.0f));

            char FPS_buffer[256];
            _snprintf_s(FPS_buffer, sizeof(FPS_buffer), 
                        " %.02fms/f, %.02ff/s, %.02fmc/f\n", ms_per_frame, fps, mega_cycles_per_frame); 
            OutputDebugStringA(FPS_buffer);
            printf(FPS_buffer);
        }
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
            // TODO: Remove?
            /*
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
            */
        } break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(window, &ps);
                Win32_Window_Dimension dimension = win32_getWindowDimension(window);
                win32_displayBufferInWindow(&global_back_buffer, hdc, dimension.width, dimension.height);

                EndPaint(window, &ps);
            }
        default: {
            result = DefWindowProcA(window, uMsg, wParam, lParam);
        }
    }
    return result;
}
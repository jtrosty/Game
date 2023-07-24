#if !defined(WIN32_CORE_H)

struct Win32_Sound_Output
{
    int samples_per_second;
    uint32 running_sample_index;
    int bytes_per_sample;
    DWORD secondary_buffer_size;
    DWORD safety_bytes;
    real32 t_sine;
    // TODO(casey): Should running sample index be in bytes as well
    // TODO(casey): Math gets simpler if we add a "bytes per second" field?
};

struct Win32_Game_Code {
    HMODULE game_code_dll;
    FILETIME dll_last_write_time;

    game_update_and_render* update_and_render_fn;
    game_get_sound_samples* get_sound_samples_fn;

    bool32 is_valid;
};

struct Win32_Debug_Time_Marker {
    DWORD output_play_cursor;
    DWORD output_write_cursor;
    DWORD output_location;
    DWORD output_byte_count;
    DWORD expected_flip_play_cursor;
    
    DWORD flipe_play_cursor;
    DWORD flip_write_cursor;
};

#define WIN32_CORE_H
#endif

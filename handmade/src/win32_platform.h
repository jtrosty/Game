
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

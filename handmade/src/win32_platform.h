
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
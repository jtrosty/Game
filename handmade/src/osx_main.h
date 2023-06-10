// By Theodore William Bendixson 
//#import <AudioToolbox/AudioToolbox.h>
//#include "../cpp/code/handmade_types.h"
//#include "../cpp/code/handmade.h"
#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint32_t bool32;

typedef float real32;

#define MAC_MAX_FILENAME_SIZE 4096

const uint16 left_arrow_key_code = 0x7B;
const uint16 right_arrow_key_code = 0x7C;
const uint16 down_arrow_key_code = 0x7D;
const uint16 up_arrow_key_code = 0x7E;
const uint16 A_key_code = 0x00;
const uint16 S_key_code = 0x01;
const uint16 D_key_code = 0x02;
const uint16 F_key_code = 0x03;
const uint16 Q_key_code = 0x0C;
const uint16 R_key_code = 0x0F;
const uint16 L_key_code = 0x25;

struct Game_Offscreen_buffer {
    uint8 bytes_per_pixel;
};

struct Game_Memory {
    /*
    GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
    GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
    GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
    Megabytes permanent_storage_size; 
    Gigabytes transient_storage_size; 
    */
    uint64 permanent_storage_size; 
    uint64 transient_storage_size; 
};

// TODO: (Ted) Have a way to specify if controller is analog.
//struct mac_game_controller
struct OSX_Game_Controller
{
    uint32 left_button_usage_ID;
    uint32 top_button_usage_ID;
    uint32 bottom_button_usage_ID;
    uint32 right_button_usage_ID;
    uint32 left_shoulder_button_usage_ID;
    uint32 right_shoulder_button_usage_ID;

    uint32 left_thumb_X_usage_ID;
    uint32 left_thumb_Y_usage_ID;

    bool32 left_button_state;
    bool32 top_button_state;
    bool32 bottom_button_state;
    bool32 right_button_state;
    bool32 left_shoulder_button_state;
    bool32 right_shoulder_button_state;

    real32 left_thumbstick_x;
    real32 left_thumbstick_y;
    
    i32 d_pad_x;
    i32 d_pad_y;

};

struct OSX_SoundOutput
{
    uint32 samples_per_second;
    uint32 bytes_per_sample;
    uint32 buffer_size;
    //
    // NOTE: (ted)  This isn't the real sound card play cursor.
    //              it's just the last time Core Audio / Apple called us.
    uint32 play_cursor;
    void *data;

    // TODO: Figure out audio component.

    /*
    AudioComponentInstance *audio_unit;
    */
};

struct OSX_Game_Code
{
    /*
    void *game_code_dll;
    time_t dll_last_write_time;
    game_update_and_render *update_and_render;
    game_get_sound_samples *get_sound_samples;
    bool32 is_valid;
    */
};

struct OSX_AppPath
{
    char filename[MAC_MAX_FILENAME_SIZE];
    char *one_past_last_app_file_name_slash;
};

// NOTE: (Ted)  This is Mac platform-specific state
struct OSX_State
{
    void* game_memory_block;
    uint64 permanent_storage_size;

    FILE* replay_file_handle;
    char replay_file_name[MAC_MAX_FILENAME_SIZE];
    void* replay_memory_block;

    OSX_AppPath path;

    FILE* recording_handle;
    bool32 is_recording;

    FILE* playback_handle;
    bool32 is_playing_back;

	char resources_directory[MAC_MAX_FILENAME_SIZE];
	int resources_directory_size;
};
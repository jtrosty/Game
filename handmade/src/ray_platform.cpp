#include "../core.h"
#include <cstdio>
#include <time.h>

#if !defined(WIN_RAY)
#define WIN_RAY 0
#endif

#if !WIN_RAY
#include <dlfcn.h>
#include <sys/syslimits.h> //TODO: I only have this for max file path macro?
#include <sys/stat.h>
#include "../../external/raylib.h"
#define RAY_STATE_FILE_NAME_COUNT PATH_MAX

#else 
#define RAY_STATE_FILE_NAME_COUNT 4096
#include "C:/raylib/raylib/src/raylib.h"
#endif

//#include "../dynamic.h"

//
//
// 2. setup repaly 
// 4. Test dynamic loading in debugger
// 5. test replay
// : testing
//
//
//kk


// NOTE: Gamepad name ID depends on drivers and OS
#define XBOX360_LEGACY_NAME_ID  "Xbox Controller"
#if defined(PLATFORM_RPI)
    #define XBOX360_NAME_ID     "Microsoft X-Box 360 pad"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#else
    #define XBOX360_NAME_ID     "Xbox 360 Controller"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#endif

const unsigned int screenWidth = 800;
const unsigned int screenHeight = 450;

int MAX_CONTROLLER_COUNT = 4;


struct Ray_Window_Dimension{
    int width;
    int height;
};


struct Ray_Replay_Buffer {
    void* file_handle;
    void* memory_map;
    char file_name[RAY_STATE_FILE_NAME_COUNT];
    void* memory_block;
};

struct Ray_State {
    uint64 total_size;
    void* game_memory_block;
    Ray_Replay_Buffer replay_buffers[4];
    
    void* recording_handle;
    int input_recording_index;

    void* playback_handle;
    int input_playing_index;
    
    char EXE_file_name[RAY_STATE_FILE_NAME_COUNT];
    char* one_past_last_EXE_filename_slash;

    long last_time_dll_loaded;
};

struct Ray_Game_Memory
{
    bool32 is_initialized;

    u64 permanent_storage_size;
    void* permanent_storage; // NOTE(casey): REQUIRED to be cleared to zero at startup

    u64 transient_storage_size;
    void* transient_storage; // NOTE(casey): REQUIRED to be cleared to zero at startup
};

struct ray_global_buffer {
    Image image;
};

struct Ray_Game_Code {
    void* game_code_dll;
    long dll_last_write_time;
    unsigned short temp_dll_number;

    game_update_and_render* update_and_render_fn;
    game_get_sound_samples* get_sound_samples_fn;

    bool32 is_valid;
};

long getLastWriteTime(char* game_lib_full_path) {
    time_t result = 0;

    return result;
}

// TODO: Not tested
static bool mac_checkDllChanges(Ray_Game_Code* game_code,
                                const char* game_dll_full_path) {
    long last_write_time = GetFileModTime(game_dll_full_path);

    if (last_write_time > game_code->dll_last_write_time) return true;

    return false;
}

static long 
mac_getFileModTime(const char* filename) {
    struct stat stat_test = {0};
    long result = 0;

    if (stat(filename, &stat_test) == 0)
    {
        time_t mod = stat_test.st_mtime;

        result = (long)mod;
    }
    return 0;
}


/*
#define GAME_UPDATE_AND_RENDER(name) void name(Thread_Context* thread, Game_Memory* memory, Game_Input* input, Game_Offscreen_Buffer* buffer)

#define GAME_GET_SOUND_SAMPLES(name) void name(Thread_Context* thread, Game_Memory* memory, Game_Sound_Output_Buffer* sound_buffer)
*/


struct Ray_Sound_Buffer {
    int samples_per_second;
    uint32 running_sample_index;
    int bytes_per_sample;
    int secondary_buffer_size;
    int safety_bytes; // probably not needed
    real32 t_sine;
    // TODO: Should running sample index be in bytes as well
    // TODO: Math gets simpler if we add a "bytes per second" field?
};

int 
main(int argv, char* argc[]) {
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Set MSAA 4X hint before windows creation

    InitWindow(screenWidth, screenHeight, "raylib [core] example");
    InitAudioDevice();

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    
    // Sound
    Music music = {0};
    AudioStream audio_stream = {0};
    audio_stream.buffer = 0;
    audio_stream.channels = 0;
    audio_stream.processor = 0;
    audio_stream.sampleRate = 0;
    audio_stream.sampleSize = 0;
    Sound sound = {0};
    sound.stream = audio_stream;
    sound.frameCount = {0};
    Game_Sound_Output_Buffer sound_buffer = {};
    sound_buffer.samples = 0;
    sound_buffer.sample_count = 0;
    sound_buffer.samples_per_second = 0;
    


    Ray_Game_Code game_code = {0};
    Ray_State ray_state = {0};
    Thread_Context thread = {0};
    thread.place_holder = 0;

    // Controllers
    Game_Input game_input[2] = {0};
    Game_Input* new_input = &game_input[0];
    Game_Input* old_input = &game_input[1];

    // Memory
    void* base_address = 0;
    Game_Memory game_memory = {0};
    game_memory.permanent_storage_size = Kilobytes(32);
    game_memory.transient_storage_size = Kilobytes(32);
    
    // TODO is the size correct here
    ray_state.total_size = game_memory.permanent_storage_size + game_memory.transient_storage_size;
    ray_state.game_memory_block = MemRealloc(base_address, ray_state.total_size);
    game_memory.permanent_storage = ray_state.game_memory_block;
    game_memory.transient_storage = ((u8*)game_memory.permanent_storage + game_memory.permanent_storage_size);

    // Getting how to transfer offscreen buffer into image on raylib
    Game_Offscreen_Buffer buffer = {0};
    buffer.height = screenHeight;
    buffer.width = screenWidth;
    buffer.bytes_per_pixel = 4;
    buffer.pitch = screenWidth * buffer.bytes_per_pixel;
    // Allocate memory for offscreen bufffer, 
    buffer.memory = MemAlloc(buffer.width * buffer.height * buffer.bytes_per_pixel);
    if (buffer.memory) {
        // success
    //
    } else {
        // Failure ot get memory allocated for image
        TraceLog(1, "Failure to allocate memory for offscreen buffer.");
    }
    Texture2D render_texture;
    render_texture.width = buffer.width;
    render_texture.height = buffer.height;
    render_texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
    render_texture.mipmaps = 1;

    // Load game code
    //--------------------------------------------------------------------------------------

#if WIN_RAY
    const char* game_dll_full_path = "w:/build/core.dll";
    const char* temp_dll_full_path = "w:/build/temp_core.dll";

#else 
    const char* game_dll_full_path = "/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/ray_core.dylib";
    //const char* game_dll_full_path = "ray_core.dylib";
    //const char* game_dll_full_path = "/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/test.txt";
    const char* temp_dll_full_path = "/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/temp_ray_core.dylib";
    
    //char* hello = LoadFileText("../test.txt");
    SaveFileText("/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/test.txt", "adding this test\0");
    // TODO does this work as expected  
    game_code = *mac_LoadDllGameCode(game_dll_full_path, temp_dll_full_path);
#endif

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Load Game Code
        //----------------------------------------------------------------------------------
#if WIN_RAY
        long last_write_time = GetFileModTime(game_dll_full_path);

#else
        if (mac_checkDllChanges(&game_code, game_dll_full_path)) {
            mac_unloadGameCode(&game_code);
            game_code = *mac_LoadDllGameCode(game_dll_full_path, temp_dll_full_path);
        }
#endif
        //----------------------------------------------------------------------------------
        
        //Controller
        //----------------------------------------------------------------------------------
        for (int controller_index = 0; controller_index < MAX_CONTROLLER_COUNT; controller_index++) {
            if (IsGamepadAvailable(controller_index)) {
                new_input->controllers[controller_index].is_connected = true;
                new_input->controllers[controller_index].is_analog = true;

                new_input->controllers[controller_index].back.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_MIDDLE_LEFT);
                new_input->controllers[controller_index].start.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_MIDDLE_RIGHT);

                new_input->controllers[controller_index].action_up.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_UP);
                new_input->controllers[controller_index].action_down.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
                new_input->controllers[controller_index].action_left.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
                new_input->controllers[controller_index].action_right.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

                new_input->controllers[controller_index].move_up.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_UP);
                new_input->controllers[controller_index].move_down.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
                new_input->controllers[controller_index].move_left.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
                new_input->controllers[controller_index].move_right.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

                new_input->controllers[controller_index].left_shoulder.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
                new_input->controllers[controller_index].right_shoulder.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);

                new_input->controllers[controller_index].left_stick_average_x = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_LEFT_X);
                new_input->controllers[controller_index].left_stick_average_y = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_LEFT_Y);

                new_input->controllers[controller_index].right_stick_average_x = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_RIGHT_X);
                new_input->controllers[controller_index].right_stick_average_y = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_RIGHT_Y);
            }
        }
        //----------------------------------------------------------------------------------
        
        // Update
        //----------------------------------------------------------------------------------
        if (game_code.get_sound_samples_fn) {
            game_code.get_sound_samples_fn(&thread, &game_memory, &sound_buffer);
        }
        if (game_code.update_and_render_fn) {
            game_code.update_and_render_fn(&thread, &game_memory, new_input, &buffer);
        }
        // TODO make sure the data is transfered properly
        UpdateTexture(render_texture, &buffer.memory);
        

        // 
        //----------------------------------------------------------------------------------
        
         
        
        //----------------------------------------------------------------------------------
        
        // Sound
        //----------------------------------------------------------------------------------
        
         
        
        //----------------------------------------------------------------------------------
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexture(render_texture, 0, 0, WHITE);

            DrawText(TextFormat("DETECTED AXIS [%i]:", GetGamepadAxisCount(0)), 10, 50, 10, MAROON);

            for (int i = 0; i < GetGamepadAxisCount(0); i++) {
                DrawText(TextFormat("AXIS %i: %.02f", i, GetGamepadAxisMovement(0, i)), 20, 70 + 20*i, 10, DARKGRAY);
            }

            if (GetGamepadButtonPressed() != GAMEPAD_BUTTON_UNKNOWN) DrawText(TextFormat("DETECTED BUTTON: %i", GetGamepadButtonPressed()), 10, 430, 10, RED);
    
            else DrawText("DETECTED BUTTON: NONE", 10, 430, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization

    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}


#include "../../external/raylib.h"
#include "../core.h"
#include <dlfcn.h>
#include <time.h>


// NOTE: Gamepad name ID depends on drivers and OS
#define XBOX360_LEGACY_NAME_ID  "Xbox Controller"
#if defined(PLATFORM_RPI)
    #define XBOX360_NAME_ID     "Microsoft X-Box 360 pad"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#else
    #define XBOX360_NAME_ID     "Xbox 360 Controller"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#endif

const int screenWidth = 800;
const int screenHeight = 450;

int MAX_CONTROLLER_COUNT = 4;

struct Ray_Offscreen_Buffer {
    // NOT;E(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    Image* image;
    void* memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

struct ray_global_buffer {
    Image image;
};

struct Ray_Game_Code {
    void* game_code_dll;
    long dll_last_write_time;

    game_update_and_render* update_and_render_fn;
    game_get_sound_samples* get_sound_samples_fn;

    bool32 is_valid;
};

long getLastWriteTime(char* game_lib_full_path) {
    time_t result = 0;

    return result;
}

// TODO: Not tested
static Ray_Game_Code* mac_LoadDllGameCode(const char* game_dll_full_path, const char* temp_dll_full_path) {

    Ray_Game_Code* result = {};

    result->dll_last_write_time = GetFileModTime(game_dll_full_path);

    // Time to copy file
    unsigned int loaded_dll_bytes_read = 0;
    
    unsigned char* loaded_dll = LoadFileData(game_dll_full_path, &loaded_dll_bytes_read);       // Load file data as byte array (read)
    if (!SaveFileData(temp_dll_full_path, loaded_dll, loaded_dll_bytes_read)) {
        
    }   // Save data to file from byte array (write), returns true on success

    // Use the copied dll for running so that way the original is available for writing when rebuilding
    result->game_code_dll = dlopen(temp_dll_full_path, RTLD_NOW);

    if (result->game_code_dll) {
        result->update_and_render_fn = (game_update_and_render*)dlsym(result->game_code_dll, "GameUpdateAndRender");
        result->get_sound_samples_fn = (game_get_sound_samples*)dlsym(result->game_code_dll, "GameGetSounddSamples");
        result->is_valid = (result->update_and_render_fn && result->get_sound_samples_fn);
    }

    // If there is a failure, set the functions pointers to zero.
    if (!result->is_valid) {
        result->update_and_render_fn = 0;
        result->get_sound_samples_fn = 0;
    }

    return result;
}

// Not Tested
static void mac_unloadGameCode(Ray_Game_Code* game_code) {
    if (game_code->game_code_dll) {
        dlclose(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }
    game_code->is_valid = false;
    game_code->update_and_render_fn = 0;
    game_code->get_sound_samples_fn = 0;
}

/*
#define GAME_UPDATE_AND_RENDER(name) void name(Thread_Context* thread, Game_Memory* memory, Game_Input* input, Game_Offscreen_Buffer* buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

// NOTE(casey): At the moment, this has to be a very fast function, it cannot be
// more than a millisecond or so.
// TODO(casey): Reduce the pressure on this function's performance by measuring it
// or asking about it, etc.
#define GAME_GET_SOUND_SAMPLES(name) void name(Thread_Context* thread, Game_Memory* memory, Game_Sound_Output_Buffer* sound_buffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
*/


int main(int argv, char* argc[]) {
    // Initialization
    //--------------------------------------------------------------------------------------
    Image test;
    ray_global_buffer buffer;
    Ray_Game_Code game_code = {0};


    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Set MSAA 4X hint before windows creation

    InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad input");


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Game_Controller_Input controllers[4] = {0};

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        // Load Game Code
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------

        // Update
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------
        //
        //Controller

        for (int controller_index = 0; controller_index < MAX_CONTROLLER_COUNT; controller_index++) {
            if (IsGamepadAvailable(controller_index)) {
                controllers[controller_index].is_connected = true;
                controllers[controller_index].is_analog = true;

                controllers[controller_index].back.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_MIDDLE_LEFT);
                controllers[controller_index].start.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_MIDDLE_RIGHT);

                controllers[controller_index].action_up.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_UP);
                controllers[controller_index].action_down.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
                controllers[controller_index].action_left.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
                controllers[controller_index].action_right.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

                controllers[controller_index].move_up.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_UP);
                controllers[controller_index].move_down.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
                controllers[controller_index].move_left.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
                controllers[controller_index].move_right.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

                controllers[controller_index].left_shoulder.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
                controllers[controller_index].right_shoulder.ended_down = IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
                
                controllers[controller_index].left_stick_average_x = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_LEFT_X);
                controllers[controller_index].left_stick_average_y = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_LEFT_Y);

                controllers[controller_index].right_stick_average_x = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_RIGHT_X);
                controllers[controller_index].right_stick_average_y = (real32)GetGamepadAxisMovement(controller_index, GAMEPAD_AXIS_RIGHT_Y);
                
            }
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);


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

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}


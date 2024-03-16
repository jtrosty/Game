#if defined(WIN_RAY)
#include "../../external/raylib.h"
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/syslimits.h> //TODO: I only have this for max file path macro?
#define RAY_STATE_FILE_NAME_COUNT PATH_MAX
#else
#define RAY_STATE_FILE_NAME_COUNT 4096
#endif

// #include "../dynamic.h"
//  #include "../../external/raylib.h"
#include "../core.h"
#include <cstdio>
#include <dlfcn.h>
#include <raylib.h>
#include <sys/stat.h>
#include <sys/syslimits.h> //TODO: I only have this for max file path macro?
#include <time.h>

//
//
// 2. setup repaly
// 4. Test dynamic loading in debugger
// 5. test replay
// : testing
//
//

// NOTE: Gamepad name ID depends on drivers and OS
#define XBOX360_LEGACY_NAME_ID "Xbox Controller"
#if defined(PLATFORM_RPI)
#define XBOX360_NAME_ID "Microsoft X-Box 360 pad"
#define PS3_NAME_ID "PLAYSTATION(R)3 Controller"
#else
#define XBOX360_NAME_ID "Xbox 360 Controller"
#define PS3_NAME_ID "PLAYSTATION(R)3 Controller"
#endif

const unsigned int LOG_VALUE = 1;

const unsigned int screenWidth = 1280;
const unsigned int screenHeight = 720;

int MAX_CONTROLLER_COUNT = 4;
/*
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(Thread_Context* thread,
void* memory) typedef
DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) Debug_Read_File_Result
name(Thread_Context* Thread, char* filename) typedef
DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(Thread_Context*
thread, char* filename, uint32 memory_size, void* memory) typedef
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
*/

//**************************** DEBUG FILE
// SECTION******************************//

static void winToRaylibPixelFormat(Game_Offscreen_Buffer* buffer) {
  // b g r a
  //    a b g r
  // Raylib is    R8 G8 B8 A8
  // Windows is   A8 R8 G8 B8
  // Comes in
  // 1  2  3  4
  //       g
  //       b
  int num_of_pixels = (buffer->height * buffer->width);
  u32* pixels = (u32*)buffer->memory;
  // u32 alpha_mask = 0x00FFFFFF;
  // u32 alpha_mask = 0xFFFFFF00;
  // u32 alpha_mask = 0x000000FF;
  u32 first_mask = 0xff000000;
  u32 secon_mask = 0x00ff0000;
  u32 third_mask = 0x0000ff00;
  u32 fourt_mask = 0x000000ff;

  u32 alpha_mask = 0xFF000000;
  for (int i = 0; i < num_of_pixels; i++) {
    //*pixel = 0xFF0000ff + x_offset++;

    u32 pixel = pixels[i];

    u32 first_byte = (pixel & first_mask) >> 24;
    u32 secon_byte = (pixel & secon_mask) >> 16;
    u32 third_byte = (pixel & third_mask) >> 8; // g
    u32 fourt_byte = (pixel & fourt_mask) >> 0;

    pixels[i] =
        alpha_mask | (fourt_byte << 16) | (third_byte << 8) | (secon_byte << 0);

    // pixels[i] = (pixel >> 8) | alpha_mask;
    //  0xFF000000 black
    //  0xFFFF0000 orange maybe red
    //  0x00FF0000 red
    //  0x0000FF00 green
    //  0x000000FF blue
  }
}

static void catStrings(size_t source_a_count, char* source_a,
                       size_t source_b_count, char* source_b, size_t dest_count,
                       char* dest) {
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

DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platformFreeFileMemory){InvalidCodePath}

DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platformReadEntireFile) {

  char* start_of_path =
      "/Users/jonathantrost/Documents/code/warfare/Game/handmade/";
  char* c = start_of_path;
  u32 length_path_start = 0;
  while (*c != '\0') {
    c++;
    length_path_start++;
  }

  Debug_Read_File_Result result = {};
  char* char_ptr = filename;
  u32 length_path = 0;
  while (*char_ptr != '\0') {
    char_ptr++;
    length_path++;
  }
  u32 final_len = length_path_start + length_path;
  char path_final[final_len + 1];
  for (u32 i = 0; i < length_path_start; i++) {
    path_final[i] = start_of_path[i];
  }
  for (u32 i = length_path_start; i < final_len; i++) {
    path_final[i] = filename[i - length_path_start + 3];
  }
  path_final[final_len] = '\0';

  result.contents =
      (void*)LoadFileData(path_final, (int*)&result.contents_size);

  return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platformWriteEntireFile) {
  bool32 result = SaveFileData(filename, memory, memory_size);

  return result;
}

//******************************** END DEBUG
//************************************//

struct Ray_Window_Dimension {
  int width;
  int height;
};

struct Ray_Sound_Output_Buffer {
  i32 samples_per_second;
  i32 sample_count;
  i16* samples;
  u32 running_sample_index;
  int bytes_per_sample;
  u32 secondary_buffer_size;
  u32 safety_bytes;
  real32 t_sine;
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

struct Ray_Game_Memory {
  bool32 is_initialized;

  u64 permanent_storage_size;
  void* permanent_storage; // NOTE(casey): REQUIRED to be cleared to zero at
                           // startup

  u64 transient_storage_size;
  void* transient_storage; // NOTE(casey): REQUIRED to be cleared to zero at
                           // startup
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
// Global
Ray_Game_Code global_game_code = {0};

long getLastWriteTime(char* game_lib_full_path) {
  time_t result = 0;

  return result;
}

// TODO: Not tested
static bool mac_checkDllChanges(Ray_Game_Code* game_code,
                                const char* game_dll_full_path) {
  long last_write_time = GetFileModTime(game_dll_full_path);

  if (last_write_time > game_code->dll_last_write_time)
    return true;

  return false;
}

static long mac_getFileModTime(const char* filename) {
  struct stat stat_test = {0};
  long result = 0;

  if (stat(filename, &stat_test) == 0) {
    time_t mod = stat_test.st_mtime;

    result = (long)mod;
  }
  return 0;
}

static void mac_LoadDllGameCode(Ray_Game_Code* game_code,
                                const char* game_dll_full_path,
                                const char* temp_dll_full_path) {

  game_code->dll_last_write_time = GetFileModTime(game_dll_full_path);
  if (game_code->dll_last_write_time) {
    printf("We got the result\n");
  }

  // Time to copy file
  unsigned int loaded_dll_bytes_read = 0;

  // Load file data as byte array (read)
  unsigned char* loaded_dll =
      LoadFileData(game_dll_full_path, (int*)&loaded_dll_bytes_read);
  if (!SaveFileData(temp_dll_full_path, loaded_dll, loaded_dll_bytes_read)) {
    printf("Failed to save temp dll\n");
    TraceLog(LOG_VALUE, "Failed to save teh temp dll.");
  } // Save data to file from byte array (write), returns true on success

  // Use the copied dll for running so that way the original is available for
  // writing when rebuilding
  // game_code->game_code_dll = dlopen(temp_dll_full_path, RTLD_NOW);
  game_code->game_code_dll = dlopen(temp_dll_full_path, RTLD_NOW);

  if (game_code->game_code_dll) {
    game_code->update_and_render_fn = (game_update_and_render*)dlsym(
        game_code->game_code_dll, "GameUpdateAndRender");
    game_code->get_sound_samples_fn = (game_get_sound_samples*)dlsym(
        game_code->game_code_dll, "GameGetSoundSamples");
    game_code->is_valid =
        (game_code->update_and_render_fn && game_code->get_sound_samples_fn);
  } else {
    printf("Failed to load game code dll\n");
    TraceLog(LOG_VALUE, "Failed to load game code dll");
  }

  // If there is a failure, set the functions pointers to zero.
  // i
  if (!game_code->is_valid) {
    game_code->update_and_render_fn = 0;
    game_code->get_sound_samples_fn = 0;
    TraceLog(LOG_VALUE, "Both game code dlls were not loaded.");
  }
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

#define MAX_SAMPLES 1024
#define MAX_SAMPLES_PER_UPDATE 8192

// DEBUG ************************8
// Cycles per second (hz)
float frequency = 440.0f;

// Audio frequency, for smoothing
float audioFrequency = 440.0f;

// Previous value, used to test if sine needs to be rewritten, and to smoothly
// modulate frequency
float oldFrequency = 1.0f;

// Index for audio rendering
float sineIdx = 0.0f;
// DEBUG ************************8
//
struct Global_Audio_Callback {
  Thread_Context* thread;
  Game_Memory* game_memory;
  Game_Sound_Output_Buffer* secondary_sound_buffer;

} global_audio_callback;

void DEBUG_AudioInputCallback(void* buffer, unsigned int frames) {
  global_audio_callback.secondary_sound_buffer->sample_count = frames;
  global_game_code.get_sound_samples_fn(
      global_audio_callback.thread, global_audio_callback.game_memory,
      global_audio_callback.secondary_sound_buffer);
  short* d = (short*)buffer;
  short* in = (short*)global_audio_callback.secondary_sound_buffer->samples;

  for (int i = 0; i < frames; i++) {
    *d++ = *in++;
    *d++ = *in++;
  }
}

int main(int argv, char* argc[]) {
  // Initialization
  //--------------------------------------------------------------------------------------
  SetConfigFlags(FLAG_MSAA_4X_HINT); // Set MSAA 4X hint before windows creation

  InitWindow(screenWidth, screenHeight, "raylib [core] example");

  SetTargetFPS(30); // Set our game to run at 60 frames-per-second

  // Sound
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

  int samples_per_second = 44100; // 2 channels
  int samples_per_frame = samples_per_second / 30;
  AudioStream stream = LoadAudioStream(samples_per_second, 16, 2);
  PlayAudioStream(stream);

  short* write_buffer =
      (short*)MemAlloc(sizeof(short) * MAX_SAMPLES_PER_UPDATE);
  SetAudioStreamVolume(stream, 1.0f);

  Ray_Sound_Output_Buffer platform_sound_buffer = {};
  platform_sound_buffer.samples_per_second = samples_per_second;
  platform_sound_buffer.t_sine = 0.0f;
  platform_sound_buffer.safety_bytes = 0;
  platform_sound_buffer.bytes_per_sample = sizeof(short) * 2;

  Game_Sound_Output_Buffer secondary_sound_buffer = {};
  secondary_sound_buffer.sample_count = 0;
  secondary_sound_buffer.samples_per_second =
      platform_sound_buffer.samples_per_second;
  secondary_sound_buffer.samples =
      (short*)MemAlloc(secondary_sound_buffer.samples_per_second *
                       platform_sound_buffer.bytes_per_sample);

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
  game_memory.permanent_storage_size = Megabytes(32);
  game_memory.transient_storage_size = Megabytes(32);
  game_memory.DEBUG_platformReadEntireFile = debug_platformReadEntireFile;
  game_memory.DEBUG_platformFreeFileMemory = debug_platformFreeFileMemory;
  game_memory.DEBUG_platformWriteEntireFile = debug_platformWriteEntireFile;

  // TODO is the size correct here
  ray_state.total_size =
      game_memory.permanent_storage_size + game_memory.transient_storage_size;
  ray_state.game_memory_block = MemRealloc(base_address, ray_state.total_size);
  game_memory.permanent_storage = ray_state.game_memory_block;
  game_memory.transient_storage =
      ((u8*)game_memory.permanent_storage + game_memory.permanent_storage_size);

  // Getting how to transfer offscreen buffer into image on raylib
  Game_Offscreen_Buffer buffer = {0};
  buffer.height = screenHeight;
  buffer.width = screenWidth;
  buffer.bytes_per_pixel = 4;
  buffer.pitch = screenWidth * buffer.bytes_per_pixel;
  // Allocate memory for offscreen bufffer,
  buffer.memory =
      MemAlloc(buffer.width * buffer.height * buffer.bytes_per_pixel);
  if (buffer.memory) {
    // success
    //
  } else {
    // Failure ot get memory allocated for image
    TraceLog(LOG_VALUE, "Failure to allocate memory for offscreen buffer.");
  }
  Image render_image = GenImageColor(buffer.width, buffer.height, RED);
  ImageFormat(&render_image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

  Texture2D render_texture = LoadTextureFromImage(render_image);
  UpdateTexture(render_texture, buffer.memory);

  global_audio_callback.secondary_sound_buffer = &secondary_sound_buffer;
  global_audio_callback.thread = &thread;
  global_audio_callback.game_memory = &game_memory;

  // Load game code
  //--------------------------------------------------------------------------------------
  const char* game_dll_full_path = "/Users/jonathantrost/Documents/code/"
                                   "warfare/Game/handmade/build/ray_core.dylib";
  // const char* game_dll_full_path = "ray_core.dylib";
  // const char* game_dll_full_path =
  // "/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/test.txt";
  const char* temp_dll_full_path =
      "/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/"
      "temp_ray_core.dylib";

  // char* hello = LoadFileText("../test.txt");
  SaveFileText("test.txt", "adding this test\0");
  // TODO does this work as expected

  mac_LoadDllGameCode(&global_game_code, game_dll_full_path,
                      temp_dll_full_path);

  /*
#if WIN_RAY
    const char* game_dll_full_path = "w:/build/core.dll";
    const char* temp_dll_full_path = "w:/build/temp_core.dll";

#else
    const char* game_dll_full_path =
"/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/ray_core.dylib";
    //const char* game_dll_full_path = "ray_core.dylib";
    //const char* game_dll_full_path =
"/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/test.txt";
    const char* temp_dll_full_path =
"/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/temp_ray_core.dylib";

    //char* hello = LoadFileText("../test.txt");
    SaveFileText("/Users/jonathantrost/Documents/code/warfare/Game/handmade/build/test.txt",
"adding this test\0");
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
            game_code = *mac_LoadDllGameCode(game_dll_full_path,
temp_dll_full_path);
        }
#endif
*/

  //--------------------------------------------------------------------------------------

  SetAudioStreamCallback(stream, DEBUG_AudioInputCallback);

  // DEBUG

  // DEBUG

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Load Game Code
    //----------------------------------------------------------------------------------
    if (mac_checkDllChanges(&global_game_code, game_dll_full_path)) {
      mac_unloadGameCode(&global_game_code);
      mac_LoadDllGameCode(&global_game_code, game_dll_full_path,
                          temp_dll_full_path);
    }
    //----------------------------------------------------------------------------------

    // Controller
    //----------------------------------------------------------------------------------
    // TODO: Hard coded zero.
    new_input->dt_for_frame = GetFrameTime();
    for (int controller_index = 0; controller_index < MAX_CONTROLLER_COUNT;
         controller_index++) {
      if (IsGamepadAvailable(0)) {
        new_input->controllers[controller_index].is_connected = true;
        new_input->controllers[controller_index].is_analog = true;

        new_input->controllers[controller_index].back.ended_down =
            IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_MIDDLE_LEFT);
        new_input->controllers[controller_index].start.ended_down =
            IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_MIDDLE_RIGHT);

        new_input->controllers[controller_index].action_up.ended_down =
            IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_RIGHT_FACE_UP);
        new_input->controllers[controller_index].action_down.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        new_input->controllers[controller_index].action_left.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
        new_input->controllers[controller_index].action_right.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

        new_input->controllers[controller_index].move_up.ended_down =
            IsGamepadButtonDown(controller_index, GAMEPAD_BUTTON_LEFT_FACE_UP);
        new_input->controllers[controller_index].move_down.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        new_input->controllers[controller_index].move_left.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        new_input->controllers[controller_index].move_right.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

        new_input->controllers[controller_index].left_shoulder.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_LEFT_TRIGGER_1);
        new_input->controllers[controller_index].right_shoulder.ended_down =
            IsGamepadButtonDown(controller_index,
                                GAMEPAD_BUTTON_RIGHT_TRIGGER_1);

        new_input->controllers[controller_index].left_stick_average_x =
            (real32)GetGamepadAxisMovement(controller_index,
                                           GAMEPAD_AXIS_LEFT_X);
        new_input->controllers[controller_index].left_stick_average_y =
            (real32)GetGamepadAxisMovement(controller_index,
                                           GAMEPAD_AXIS_LEFT_Y);

        new_input->controllers[controller_index].right_stick_average_x =
            (real32)GetGamepadAxisMovement(controller_index,
                                           GAMEPAD_AXIS_RIGHT_X);
        new_input->controllers[controller_index].right_stick_average_y =
            (real32)GetGamepadAxisMovement(controller_index,
                                           GAMEPAD_AXIS_RIGHT_Y);
      } else {
        new_input->controllers[0].is_connected = false;
        new_input->controllers[0].is_analog = false;

        new_input->controllers[0].back.ended_down = IsKeyDown(KEY_B);
        new_input->controllers[0].start.ended_down = IsKeyDown(KEY_G);

        new_input->controllers[0].action_up.ended_down = IsKeyDown(KEY_Q);
        new_input->controllers[0].action_down.ended_down = IsKeyDown(KEY_E);
        new_input->controllers[0].action_left.ended_down = IsKeyDown(KEY_F);
        new_input->controllers[0].action_right.ended_down = IsKeyDown(KEY_R);

        new_input->controllers[0].move_up.ended_down = IsKeyDown(KEY_W);
        new_input->controllers[0].move_down.ended_down = IsKeyDown(KEY_S);
        new_input->controllers[0].move_left.ended_down = IsKeyDown(KEY_A);
        new_input->controllers[0].move_right.ended_down = IsKeyDown(KEY_D);

        new_input->controllers[0].left_shoulder.ended_down = IsKeyDown(KEY_X);
        new_input->controllers[0].right_shoulder.ended_down = IsKeyDown(KEY_Z);
      }
    }
    //----------------------------------------------------------------------------------

    // Update
    //----------------------------------------------------------------------------------
    /*
    if (global_game_code.get_sound_samples_fn) {
        global_game_code.get_sound_samples_fn(&thread, &game_memory,
    &secondary_sound_buffer);
    }
    */
    if (global_game_code.update_and_render_fn) {
      global_game_code.update_and_render_fn(&thread, &game_memory, new_input,
                                            &buffer);
    }
    // TODO make sure the data is transfered properly
    winToRaylibPixelFormat(&buffer);
    UpdateTexture(render_texture, buffer.memory);

    //
    //----------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------

    // Sound
    //----------------------------------------------------------------------------------
    // if (IsAudioStreamProcessed(stream)) {
    //    UpdateAudioStream(stream, &secondary_sound_buffer.samples,
    //    MAX_SAMPLES_PER_UPDATE);
    //}

    //----------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawTexture(render_texture, 0, 0, WHITE);
    DrawCircleLines(400, 400, 100.0f, RED);

    DrawText(TextFormat("We need to make sure this works Trost with the most."),
             10, 50, 10, MAROON);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  MemFree(game_memory.permanent_storage);
  MemFree(write_buffer);
  MemFree(secondary_sound_buffer.samples);
  MemFree(buffer.memory);

  UnloadImage(render_image);
  UnloadTexture(render_texture);
  UnloadAudioStream(stream);

  CloseAudioDevice();
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------
  return 0;
}

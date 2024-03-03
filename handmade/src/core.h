#include <math.h>
#include <stdint.h>

#define ERROR_REMOVER

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

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
typedef double real64;

typedef size_t memory_index;

#define InvalidCodePath Assert(!"InvalidCodePath");

#if HANDMADE_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression)                                                     \
  if (!(Expression)) {                                                         \
    *(int *)0 = 0;                                                             \
  }
#else
#define Assert(Expression)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

struct Thread_Context {
  int place_holder;
};

/* IMPORTANT(casey):

   These are NOT for doing anything in the shipping game - they are
   blocking and the write doesn't protect against lost data!
*/

struct Memory_Arena {
  memory_index size;
  u8 *base;
  memory_index used;
};

static void initializeArena(Memory_Arena *arena, memory_index size, u8 *base) {
  arena->size = size;
  arena->base = base;
  arena->used = 0;
}

#define pushStruct(arena, type) (type *)pushSize_(arena, sizeof(type))
#define pushArray(arena, count, type)                                          \
  (type *)pushSize_(arena, (count) * sizeof(type))
static void *pushSize_(Memory_Arena *arena, memory_index size) {
  // All memory when initial gained from valloc is cleared to zero
  Assert((arena->used + size) <= arena->size);
  void *result = arena->base + arena->used;
  arena->used += size;

  return result;
}

#include "core_intrinsics.h"
#include "core_math.h"
#include "core_world.h"

typedef struct Debug_Read_File_Result {
  uint32 contents_size;
  void *contents;
} Debug_Read_File_Result;

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name)                                  \
  void name(Thread_Context *thread, void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name)                                  \
  Debug_Read_File_Result name(Thread_Context *Thread, char *filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name)                                 \
  bool32 name(Thread_Context *thread, char *filename, uint32 memory_size,      \
              void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

inline u32 safeTruncateU64(u64 value) {
  Assert(value <= 0xFFFFFFFF);
  u32 result = (u32)value;
  return result;
}

struct Game_Offscreen_Buffer {
  // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
  void *memory;
  int width;
  int height;
  int pitch;
  int bytes_per_pixel;
};

struct Game_Sound_Output_Buffer {
  int samples_per_second;
  int sample_count;
  i16 *samples;
};

struct Game_Button_State {
  int half_transition_count;
  bool32 ended_down;
};

struct Game_Controller_Input {
  bool32 is_connected;
  bool32 is_analog;
  real32 left_stick_average_x;
  real32 left_stick_average_y;
  real32 right_stick_average_x;
  real32 right_stick_average_y;

  union {
    Game_Button_State buttons[12];
    struct {
      Game_Button_State move_up;
      Game_Button_State move_down;
      Game_Button_State move_left;
      Game_Button_State move_right;

      Game_Button_State action_up;
      Game_Button_State action_down;
      Game_Button_State action_left;
      Game_Button_State action_right;

      Game_Button_State left_shoulder;
      Game_Button_State right_shoulder;

      Game_Button_State back;
      Game_Button_State start;

      // NOTE(casey): All buttons must be added above this line

      Game_Button_State terminator;
    };
  };
};

struct Game_Input {
  Game_Button_State mouse_buttons[5];
  i32 mouse_x, mouse_y, mouse_z;

  real32 dt_for_frame;

  // TODO(casey): Insert clock values here.
  Game_Controller_Input controllers[5];
};

inline Game_Controller_Input *getController(Game_Input *input,
                                            int unsigned controller_index) {
  Assert(controller_index < ArrayCount(input->controllers));

  Game_Controller_Input *result = &input->controllers[controller_index];
  return (result);
}

// TO
struct Game_Memory {
  bool32 is_initialized;

  u64 permanent_storage_size;
  void *permanent_storage; // NOTE(casey): REQUIRED to be cleared to zero at
                           // startup

  u64 transient_storage_size;
  void *transient_storage; // NOTE(casey): REQUIRED to be cleared to zero at
                           // startup

  debug_platform_free_file_memory *DEBUG_platformFreeFileMemory;
  debug_platform_read_entire_file *DEBUG_platformReadEntireFile;
  debug_platform_write_entire_file *DEBUG_platformWriteEntireFile;
};

#define GAME_UPDATE_AND_RENDER(name)                                           \
  void name(Thread_Context *thread, Game_Memory *memory, Game_Input *input,    \
            Game_Offscreen_Buffer *buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

// NOTE(casey): At the moment, this has to be a very fast function, it cannot be
// more than a millisecond or so.
// TODO(casey): Reduce the pressure on this function's performance by measuring
// it or asking about it, etc.
#define GAME_GET_SOUND_SAMPLES(name)                                           \
  void name(Thread_Context *thread, Game_Memory *memory,                       \
            Game_Sound_Output_Buffer *sound_buffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

struct Loaded_Bitmap {
  u32 *pixels;
  i32 width;
  i32 height;
};

struct Hero_Bitmaps {
  i32 align_x;
  i32 align_y;
  Loaded_Bitmap head;
  Loaded_Bitmap cape;
  Loaded_Bitmap torso;
};

enum Entity_Type {
  entityType_Null,

  entityType_Hero,
  entityType_Wall,
  entityType_Familiar,
  entityType_Monster,
  entityType_Sword,
};
struct High_Entity {
  v2 p; // Relative to the camera
  v2 dp;

  u32 abs_tile_z;
  u32 facingDirection;

  real32 z;
  real32 dZ;

  u32 low_entity_index;
};

struct Low_Entity {
  Entity_Type type;
  World_Position p;
  real32 width;
  real32 height;

  // NOTE: This is for stairs
  bool32 colliders;
  i32 d_abs_tile_z;
  u32 high_entity_index;
};

struct Entity {
  u32 low_index;
  Low_Entity *low;
  High_Entity *high;
};

struct Game_State {
  World *world;
  Memory_Arena world_arena;

  u32 camera_following_entity_index;
  u32 player_index_for_controllers[ArrayCount(((Game_Input *)0)->controllers)];

  u32 low_entity_count;
  Low_Entity low_entities[10000];

  u32 high_entity_count;
  High_Entity high_entities[256];

  Loaded_Bitmap backdrop;
  Loaded_Bitmap shadow;
  Loaded_Bitmap tree;
  Loaded_Bitmap rock;
  Hero_Bitmaps hero_bitmap[4];

  // Player Stuff ----------------
  // Tile_Map_Position player_pos;
  int player_speed_scaler;
  real32 player_side_length;
  v2 d_player_p;

  //-------------------------------
  World_Position camera_pos;
  Loaded_Bitmap DEBUG_bitmap;

  real32 meters_to_pixels;
  // DEBUG stuff
  int tone_hz;
  real32 t_sine;
};

struct Entity_Visible_Piece {
  Loaded_Bitmap *bitmap;
  v2 offset;
  real32 offset_z;
  real32 entity_z_c;
  real32 r, g, b, a;
  v2 dim;
};

struct Entity_Visible_Piece_Group {
  Game_State *game_state;
  u32 piece_count;
  Entity_Visible_Piece pieces[32];
};

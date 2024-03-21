// #include "sim_region.h"
#include <utility>

#define UNITY_BUILD_CLANGD_ERR_REMOVER

#include "core.h"
#include "core_entity.h"
#include "core_random.h"

#include "core_world.cpp"

#include "core_sim_region.cpp"

extern "C" {
#include "cLDtk.h"
}

internal void gameOutputSound(Game_State* game_state,
                              Game_Sound_Output_Buffer* sound_buffer,
                              int tone_hz) {

  i16 tone_volume = 1000;
  int wave_period = sound_buffer->samples_per_second / tone_hz;

  i16* sample_out = sound_buffer->samples;
  for (int sample_index = 0; sample_index < sound_buffer->sample_count;
       ++sample_index) {
    // TODO(casey): Draw this out for people
#if 1
    real32 sine_value = sinf(game_state->t_sine);
    i16 sample_value = (i16)(sine_value * tone_volume);
#else
    int16 SampleValue = 0;
#endif
    *sample_out++ = sample_value;
    *sample_out++ = sample_value;

    game_state->t_sine += 2.0f * Pi32 * 1.0f / (real32)wave_period;
    if (game_state->t_sine > 2.0f * Pi32) {
      game_state->t_sine -= 2.0f * Pi32;
    }
  }
}

internal void renderWeirdGradient(Game_Offscreen_Buffer* buffer,
                                  int blue_offset, int green_offset) {
  // TODO(casey): Let's see what the optimizer does

  uint8* rel_row = (uint8*)buffer->memory;
  for (int y = 0; y < buffer->height; ++y) {
    uint32* pixel = (uint32*)rel_row;

    for (int x = 0; x < buffer->width; ++x) {
      uint8 blue = (uint8)(x + blue_offset);
      uint8 green = (uint8)(y + green_offset);

      *pixel++ = ((green << 16) | blue);
    }

    rel_row += buffer->pitch;
  }
}

#pragma pack(push, 1)
struct Bitmap_Header {
  u16 file_type;
  u32 file_size;
  u16 reserved_1;
  u16 reserved_2;
  u32 bitmap_offset;
  u32 size;
  i32 width;
  i32 height;
  u16 planes;
  u16 bits_per_pixel;
  u32 compression;
  u32 size_of_bitmap;
  i32 horz_resolution;
  i32 vert_resolution;
  u32 colors_used;
  u32 colors_important;

  u32 red_mask;
  u32 green_mask;
  u32 blue_mask;
};
#pragma pack(pop)

static Loaded_Bitmap

DEBUG_loadBMP(debug_platform_read_entire_file* read_entire_file,
              Thread_Context* thread, char* filename) {

  // BB GG RR AA, bottom up, start with bottomr row first
  Loaded_Bitmap result = {};
  Debug_Read_File_Result read_result = read_entire_file(thread, filename);
  if (read_result.contents_size != 0) {

    Bitmap_Header* header = (Bitmap_Header*)read_result.contents;
    u32* pixels = (u32*)((u8*)read_result.contents + header->bitmap_offset);
    result.pixels = (u32*)((u8*)read_result.contents + header->bitmap_offset);

    result.width = header->width;
    result.height = header->height;

    Assert(header->compression == 3);

    // NOTE(casey): If you are using this generically for some reason,
    // please remember that BMP files CAN GO IN EITHER DIRECTION and
    // the height will be negative for top-down.
    // (Also, there can be compression, etc., etc... DON'T think this
    // is complete BMP loading code because it isn't!!)

    // NOTE(casey): Byte order in memory is determined by the Header itself,
    // so we have to read out the masks and convert the pixels ourselves.
    u32 red_mask = header->red_mask;
    u32 green_mask = header->green_mask;
    u32 blue_mask = header->blue_mask;
    u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

    Bit_Scan_Result red_scan = findLeastSignificantSetBit(red_mask);
    Bit_Scan_Result green_scan = findLeastSignificantSetBit(green_mask);
    Bit_Scan_Result blue_scan = findLeastSignificantSetBit(blue_mask);
    Bit_Scan_Result alpha_scan = findLeastSignificantSetBit(alpha_mask);

    Assert(red_scan.found);
    Assert(green_scan.found);
    Assert(blue_scan.found);
    Assert(alpha_scan.found);

    i32 red_shift = 16 - (i32)red_scan.index;
    i32 green_shift = 8 - (i32)green_scan.index;
    i32 blue_shift = 0 - (i32)blue_scan.index;
    i32 alpha_shift = 24 - (i32)alpha_scan.index;

    u32* source_dest = pixels;

    for (i32 Y = 0; Y < header->height; ++Y) {
      for (i32 X = 0; X < header->width; ++X) {
        u32 C = *source_dest;

        *source_dest++ = (rotateLeft(C & red_mask, red_shift) |
                          rotateLeft(C & green_mask, green_shift) |
                          rotateLeft(C & blue_mask, blue_shift) |
                          rotateLeft(C & alpha_mask, alpha_shift));
      }
    }
  } else {
    // TODO: Failed ot load
  }
  return result;
}

static void drawBitmap(Game_Offscreen_Buffer* buffer, Loaded_Bitmap* bitmap,

                       real32 real_x, real32 real_y, i32 align_x = 0,
                       i32 align_y = 0, real32 cAlpha = 1.0f) {
  real_x -= (real32)align_x;
  real_y -= (real32)align_y;

  i32 min_x = roundReal32ToInt32(real_x);
  i32 max_x = min_x + (real32)bitmap->width;
  i32 min_y = roundReal32ToInt32(real_y);
  i32 max_y = min_y + (real32)bitmap->height;

  i32 source_offset_x = 0;
  if (min_x < 0) {
    source_offset_x = -min_x;
    min_x = 0;
  }
  i32 source_offset_y = 0;
  if (min_y < 0) {
    source_offset_y = -min_y;
    min_y = 0;
  }
  if (max_x > buffer->width) {
    // source_offset_x = max_x;
    max_x = buffer->width;
  }
  if (max_y > buffer->height) {
    // source_offset_y = max_y;
    max_y = buffer->height;
  }
  u32* source_row = bitmap->pixels + bitmap->width * (bitmap->height - 1);
  source_row += (-source_offset_y * bitmap->width) + source_offset_x;

  u8* dest_row = ((u8*)buffer->memory + max_x * buffer->bytes_per_pixel +
                  max_y * buffer->pitch);

  for (int y = min_y; y < max_y; ++y) {
    u32* dest = (u32*)dest_row;
    u32* source = source_row;

    for (int x = min_x; x < max_x; ++x) {
      real32 A = (real32)((*source >> 24) & 0xFF) / 255.0f;
      A *= cAlpha;

      real32 SR = (real32)((*source >> 16) & 0xFF);
      real32 SG = (real32)((*source >> 8) & 0xFF);
      real32 SB = (real32)((*source >> 0) & 0xFF);

      real32 DR = (real32)((*dest >> 16) & 0xFF);
      real32 DG = (real32)((*dest >> 8) & 0xFF);
      real32 DB = (real32)((*dest >> 0) & 0xFF);

      real32 R = (1.0f - A) * DR + (A * SR);
      real32 G = (1.0f - A) * DG + (A * SG);
      real32 B = (1.0f - A) * DB + (A * SB);

      *dest = (((uint32)(R + 0.5f) << 16) | ((uint32)(G + 0.5f) << 8) |
               ((uint32)(B + 0.5f) << 0));
      ++dest;
      ++source;
    }
    dest_row -= buffer->pitch;
    source_row -= bitmap->width;
  }
}

static void drawRectangle(Game_Offscreen_Buffer* buffer, v2 v_min, v2 v_max,
                          real32 r, real32 g, real32 b) {

  i32 min_x = roundReal32ToInt32(v_min.x);
  i32 min_y = roundReal32ToInt32(v_min.y);
  i32 max_x = roundReal32ToInt32(v_max.x);
  i32 max_y = roundReal32ToInt32(v_max.y);

  if (min_x < 0) {
    min_x = 0;
  }
  if (min_y < 0) {
    min_y = 0;
  }
  if (max_x > buffer->width) {
    max_x = buffer->width;
  }
  if (max_y > buffer->height) {
    max_y = buffer->height;
  }

  u32 rel_color = ((roundReal32ToUint32(r * 255.0f) << 16) |
                   (roundReal32ToUint32(g * 255.0f) << 8) |
                   (roundReal32ToUint32(b * 255.0f) << 0));

  u8* rel_row = ((u8*)buffer->memory + min_x * buffer->bytes_per_pixel +
                 min_y * buffer->pitch);
  for (int y = min_y; y < max_y; ++y) {
    u32* pixel = (u32*)rel_row;

    for (int x = min_x; x < max_x; ++x) {
      *pixel++ = rel_color;
    }
    rel_row += buffer->pitch;
  }
}

static void renderPlayer(Game_Offscreen_Buffer* buffer, int player_x,
                         int player_y, int length_of_side) {
  u32 rel_color = 0xFF00ff00; // FFFFFF;
}

inline v2 getCameraSpaceP(Game_State* game_state, Low_Entity* entity_low) {
  World_Difference diff =
      worldSubtract(game_state->world, &entity_low->p, &game_state->camera_pos);
  v2 result = diff.dXY;

  return result;
}

inline void pushPiece(Entity_Visible_Piece_Group* group, Loaded_Bitmap* bitmap,
                      v2 offset, real32 offset_z, v2 align, v2 dim, v4 color,
                      real32 entity_z_c) {
  Assert(group->piece_count < ArrayCount(group->pieces));
  Entity_Visible_Piece* piece = group->pieces + group->piece_count++;

  piece->bitmap = bitmap;
  piece->offset =
      group->game_state->meters_to_pixels * V2(offset.x, offset.y) - align;
  piece->offset_z = group->game_state->meters_to_pixels * offset_z;
  piece->entity_z_c = entity_z_c;
  piece->r = color.r;
  piece->g = color.g;
  piece->b = color.b;
  piece->a = color.a;
  piece->dim = dim;
}

inline void pushBitmap(Entity_Visible_Piece_Group* group, Loaded_Bitmap* bitmap,
                       v2 offset, real32 offset_z, v2 align,
                       real32 alpha = 1.0f, real32 entity_z_c = 1.0f) {
  pushPiece(group, bitmap, offset, offset_z, align, V2(0, 0),
            V4(1.0f, 1.0f, 1.0f, alpha), entity_z_c);
}

inline void pushRect(Entity_Visible_Piece_Group* group, v2 offset,
                     real32 offset_z, v2 dim, v4 color,
                     real32 entity_z_c = 1.0f) {
  pushPiece(group, 0, offset, offset_z, V2(0, 0), dim, color, entity_z_c);
}

/*
inline Entity entityFromHighIndex(Game_State* game_state, u32 high_entity_index)
{ Entity result = {};

    if (high_entity_index) {
        Assert(high_entity_index < ArrayCount(game_state->high_entities));
        result.high = game_state->high_entities + high_entity_index;
        result.low_index = result.high->low_entity_index;
        result.low = game_state->low_entities + result.low_index;
    }
    return result;
}

inline High_Entity* makeEntityHighFrequency(Game_State* game_state, Low_Entity*
entity_low, u32 low_index, v2 camera_space_p) { High_Entity* entity_high = 0;

  Assert(entity_low->high_entity_index == 0);

  if (entity_low->high_entity_index == 0) {
    entity_high = game_state->high_entities + entity_low->high_entity_index;

        if (game_state->high_entity_count <
ArrayCount(game_state->high_entities)) { u32 high_index =
game_state->high_entity_count++; entity_high = game_state->high_entities +
high_index;

      entity_high->p = camera_space_p;
      entity_high->dp = V2(0, 0);
      entity_high->abs_tile_z = entity_low->p.chunk_z;
      entity_high->facingDirection = 0;
      entity_high->low_entity_index = low_index;

      entity_low->high_entity_index = high_index;
    } else {
      Assert(false);
    }
  }
  return entity_high;
}

inline High_Entity* makeEntityHighFrequency(Game_State* game_state, u32
low_index) { High_Entity* entity_high = 0;

  Low_Entity *entity_low = game_state->low_entities + low_index;

    if(entity_low->high_entity_index) {
        entity_high = game_state->high_entities + entity_low->high_entity_index;
    }
    else {
        v2 camera_space_p = getCameraSpaceP(game_state, entity_low);
        entity_high = makeEntityHighFrequency(game_state, entity_low, low_index,
camera_space_p); } return entity_high;
}

inline Entity forceEntityIntoHigh(Game_State *game_state, u32 low_index) {
  Entity result = {};

  if ((low_index > 0) && (low_index < game_state->low_entity_count)) {
    result.low_index = low_index;
    result.low = game_state->low_entities + low_index;
    result.high = makeEntityHighFrequency(game_state, low_index);
  }

  return result;
}

inline Entity getHighEntity(Game_State *game_state, u32 low_index) {
  Entity result = {};

  if ((low_index > 0) && (low_index < game_state->low_entity_count)) {
    result.low_index = low_index;
    result.low = game_state->low_entities + low_index;
    result.high = makeEntityHighFrequency(game_state, low_index);
  }
  return result;
}

inline void makeEntityLowFrequency(Game_State *game_state, u32 low_index) {
  Low_Entity *entity_low = &game_state->low_entities[low_index];
  u32 high_index = entity_low->high_entity_index;

    if(high_index) {
        u32 last_high_index = game_state->high_entity_count - 1;
        if (high_index != last_high_index) {
            High_Entity* last_entity = game_state->high_entities +
last_high_index; High_Entity* del_entity = game_state->high_entities +
high_index;

            *del_entity = *last_entity;
            game_state->low_entities[last_entity->low_entity_index].high_entity_index
= high_index;
        }

        Low_Entity* entity_low = &game_state->low_entities[low_index];
       --game_state->high_entity_count;
        entity_low->high_entity_index = 0;
    }
    --game_state->high_entity_count;
    entity_low->high_entity_index = 0;
  }
}

inline bool32 validateEntityPairs(Game_State* game_state) {
    bool32 valid = true;
    for (u32 high_entity_index = 1; high_entity_index <
game_state->high_entity_count; ++high_entity_index) { High_Entity* high =
game_state->high_entities + high_entity_index; valid = valid &&
(game_state->low_entities[high->low_entity_index].high_entity_index ==
high_entity_index);
    }
    return valid;
}

// Checks if entity has left the high freq area, moves to low feq if needed
inline void offsetAndCheckFrequencyByArea(Game_State* game_state, v2 offset,
Rectangle2 high_frequency_bounds) { for (u32 entitiy_index = 1; entitiy_index <
game_state->high_entity_count; ) { High_Entity* high = game_state->high_entities
+ entitiy_index; Low_Entity* low = game_state->low_entities +
high->low_entity_index;

        high->p += offset;
        if (isInRectangle(high_frequency_bounds, high->p) && isValid(low->p)) {
            ++entitiy_index;
        }
        else {
            Assert(game_state->low_entities[high->low_entity_index].high_entity_index
== entitiy_index); makeEntityLowFrequency(game_state, high->low_entity_index);
        }
        int dummyLine = 5;
    }
    int dummyLine = 5;
  }
}
*/

struct Add_Low_Entity_Result {
  Low_Entity* low;
  u32 low_index;
};

static Add_Low_Entity_Result
createLowEntity(Game_State* game_state, Entity_Type type, World_Position p) {
  Assert(game_state->low_entity_count < ArrayCount(game_state->low_entities));
  u32 entity_index = game_state->low_entity_count++;

  Low_Entity* entity_low = game_state->low_entities + entity_index;
  *entity_low = {};
  entity_low->sim.type = type;
  entity_low->p = nullPosition();

  changeEntityLocation(&game_state->world_arena, game_state->world,
                       entity_index, entity_low, p);

  Add_Low_Entity_Result result;
  result.low = entity_low;
  result.low_index = entity_index;

  // TODO(casey): Do we need to have a begin/end paradigm for adding
  // entities so that they can be brought into the high set when they
  // are added and are in the camera region?

  return result;
}

static Add_Low_Entity_Result addWall(Game_State* game_state, u32 abs_tile_x,
                                     u32 abs_tile_y, u32 abs_tile_z) {

  World_Position p = chunkPositionFromTilePosition(
      game_state->world, abs_tile_x, abs_tile_y, abs_tile_z);
  Add_Low_Entity_Result entity =
      createLowEntity(game_state, entityType_Wall, p);

  entity.low->sim.height = game_state->world->tile_side_in_meters;
  entity.low->sim.width = entity.low->sim.height;
  addFlag(&entity.low->sim, Entity_Flag_Collides);

  return entity;
}

static Add_Low_Entity_Result initializePlayer(Game_State* game_state) {
  World_Position p = game_state->camera_pos;
  Add_Low_Entity_Result entity =
      createLowEntity(game_state, entityType_Hero, p);

  entity.low->sim.height = 1.0f;
  entity.low->sim.width = 1.0f;
  entity.low->sim.collides = true;

  if (game_state->camera_following_entity_index == 0) {
    game_state->camera_following_entity_index = entity.low_index;
  }
  return entity;
}

static void setCamera(Game_State* game_state) {
  if (game_state->camera_following_entity_index) {
    game_state->camera_pos =
        game_state->low_entities[game_state->camera_following_entity_index].p;
  }
}

/*
    //Assert(validateEntityPairs(game_state));

    World_Difference d_camera_p = worldSubtract(world, &new_camera_p,
&game_state->camera_pos); game_state->camera_pos = new_camera_p;

    //TODO Magic numbers remove theese handle how much of
    // ***********************************
    u32 tile_span_x = 17 * 3;
    u32 tile_span_y = 9 * 3;
    // ***********************************

    Rectangle2 camera_bounds = rectCenterDim(V2(0, 0),
                                world->tile_side_in_meters *
V2((real32)tile_span_x, (real32)tile_span_y)); v2 entity_offset_for_frame =
-d_camera_p.dXY; offsetAndCheckFrequencyByArea(game_state,
entity_offset_for_frame, camera_bounds);

    World_Position min_chunk_p = mapIntoChunkSpace(world, new_camera_p,
getMinCorner(camera_bounds)); World_Position max_chunk_p =
mapIntoChunkSpace(world, new_camera_p, getMaxCorner(camera_bounds));

    for (i32 chunk_y = min_chunk_p.chunk_y; chunk_y <= max_chunk_p.chunk_y;
++chunk_y) {

        for (i32 chunk_x = min_chunk_p.chunk_x; chunk_x <= max_chunk_p.chunk_x;
++chunk_x) {

            World_Chunk* chunk = getWorldChunk(world, chunk_x, chunk_y,
new_camera_p.chunk_z); if (chunk) { for (World_Entity_Block* block =
&chunk->first_block; block; block = block->next) { for (u32 entity_index_index =
0; entity_index_index < block->entity_count; ++ entity_index_index) {

                        u32 low_entity_index =
block->low_entity_index[entity_index_index]; if (low_entity_index ==
test_entity_index) { int foo = 5;
                        }

                        Low_Entity* low = game_state->low_entities +
low_entity_index; if (low->high_entity_index == 0) { v2 camera_space_p =
getCameraSpaceP(game_state, low); if (isInRectangle(camera_bounds,
camera_space_p)) { makeEntityHighFrequency(game_state, low, low_entity_index,
camera_space_p);
                            }
                        }
                    }
                }
            }
        }
    }
    Assert(validateEntityPairs(game_state));
    // TODO: This needs to accelerated, checks every low entity if it is in area
to make high freq i32 min_tile_x = new_camera_p.abs_tile_x - (tile_span_x / 2);
    i32 max_tile_x = new_camera_p.abs_tile_x + (tile_span_x / 2);
    i32 min_tile_y = new_camera_p.abs_tile_y - (tile_span_y / 2);
    i32 max_tile_y = new_camera_p.abs_tile_y + (tile_span_y / 2);

    for (u32 entity_index = 1; entity_index < game_state->low_entity_count;
++entity_index) { Low_Entity* low_entity = game_state->low_entities +
entity_index; if (low_entity->high_entity_index == 0) { if
((low_entity->p.abs_tile_z == new_camera_p.abs_tile_z) &&
                (low_entity->p.abs_tile_x >= min_tile_x) &&
                (low_entity->p.abs_tile_x <= max_tile_x) &&
                (low_entity->p.abs_tile_y >= min_tile_y) &&
                (low_entity->p.abs_tile_y <= max_tile_y)) {
                makeEntityHighFrequency(game_state, entity_index);

            }
          }
        }
      }
    }
}
    */

// NOTE: I have changed the name to 'createPlayerEntity' form addPlayer
static Add_Low_Entity_Result createPlayerEntity(Game_State* game_state) {

  World_Position p = game_state->camera_pos;
  Add_Low_Entity_Result entity =
      createLowEntity(game_state, entityType_Hero, p);

  entity.low->sim.height = 0.5f; // 1.4f;
  entity.low->sim.width = 1.0f;
  addFlag(&entity.low->sim, Entity_Flag_Collides);
  entity.low->sim.collides = true;

  if (game_state->camera_following_entity_index == 0) {
    game_state->camera_following_entity_index = entity.low_index;
  }

  return entity;
}
///// We will see

// LDTK Loading
struct Level_Layer {
  char* identifier;
  char* type;
  int width;
  int height;
  void* tiles;
};

struct LDTK_Data {};

static void loadLevelData(debug_platform_read_entire_file* read_entire_file,
                          char* filename, Thread_Context* thread) {
  Debug_Read_File_Result read_result = read_entire_file(thread, filename);
}
///// We will see

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
  // name(Thread_Context* thread, Game_Memory* memory, Game_Input* input,
  // Game_Offscreen_Buffer* buffer)
  loadJSONFile("{\"jsonVersion\":\"\"}", "../test/test_ldtk_level.ldtk");
  importMapData();
  struct levels* lvl_one;
  lvl_one = getLevel("AutoLayer");

  Assert(sizeof(Game_State) <= memory->permanent_storage_size);
  Assert(
      (&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
      (ArrayCount(input->controllers[0].buttons)));

  Game_State* game_state = (Game_State*)memory->permanent_storage;

  if (!memory->is_initialized) {
    // TODO: Remove?

    char* filename = __FILE__;

    World_Position p_zero;
    p_zero.offset = v2{0, 0};
    p_zero.chunk_x = 1;
    p_zero.chunk_y = 1;
    p_zero.chunk_z = 0;
    createLowEntity(game_state, entityType_Null, nullPosition());

    game_state->backdrop = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile,
                                         thread, "../test/test_background.bmp");
    game_state->shadow = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile,
                                       thread, "../test/test_hero_shadow.bmp");
    game_state->tree = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile,
                                     thread, "../test/tree00.bmp");
    game_state->rock = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile,
                                     thread, "../test/rock03.bmp");
    game_state->DEBUG_bitmap =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/deub_bitmap.bmp");

    Hero_Bitmaps* hero_bitmaps;

    hero_bitmaps = game_state->hero_bitmap;
    hero_bitmaps[0].head =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_right_head.bmp");
    hero_bitmaps[0].cape =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_right_cape.bmp");
    hero_bitmaps[0].torso =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_right_torso.bmp");
    hero_bitmaps[0].align_x = 72;
    hero_bitmaps[0].align_y = 182;

    hero_bitmaps[1].head =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_back_head.bmp");
    hero_bitmaps[1].cape =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_back_cape.bmp");
    hero_bitmaps[1].torso =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_back_torso.bmp");
    hero_bitmaps[1].align_x = 72;
    hero_bitmaps[1].align_y = 182;

    hero_bitmaps[2].head =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_left_head.bmp");
    hero_bitmaps[2].cape =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_left_cape.bmp");
    hero_bitmaps[2].torso =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_left_torso.bmp");
    hero_bitmaps[2].align_x = 72;
    hero_bitmaps[2].align_y = 182;

    hero_bitmaps[3].head =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_front_head.bmp");
    hero_bitmaps[3].cape =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_front_cape.bmp");
    hero_bitmaps[3].torso =
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread,
                      "../test/test_hero_front_torso.bmp");
    hero_bitmaps[3].align_x = 72;
    hero_bitmaps[3].align_y = 182;
    /*
    Debug_Read_File_Result file = memory->DEBUG_platformReadEntireFile(thread,
    filename); if(file.contents)
    {
        memory->DEBUG_platformWriteEntireFile(thread, "test.out",
    file.contents_size, file.contents);
        memory->DEBUG_platformFreeFileMemory(thread, file.contents);
    }
    */

    initializeArena(&game_state->world_arena,
                    memory->permanent_storage_size - sizeof(Game_State),
                    (u8*)memory->permanent_storage + sizeof(Game_State));
    game_state->world = pushStruct(&game_state->world_arena, World);
    World* world = game_state->world;
    initializeWorld(world, 1.4f);

    game_state->tone_hz = 512;

    game_state->player_speed_scaler = 10.0f;

    world->tile_side_in_pixels = 60;
    real32 meters_to_pixels =
        (real32)world->tile_side_in_pixels / (real32)world->tile_side_in_meters;
    game_state->meters_to_pixels = meters_to_pixels;

    u32 random_number_index = 0;
    u32 tiles_per_width = 17;
    u32 tiles_per_height = 9;

    u32 screen_base_x = 0;
    u32 screen_base_y = 0;
    u32 screen_base_z = 0;
    u32 screen_x = screen_base_x;
    u32 screen_y = screen_base_y;
    u32 abs_tile_z = screen_base_z;

    bool32 door_left = false;
    bool32 door_right = false;
    bool32 door_top = false;
    bool32 door_bottom = false;
    bool32 door_up = false;
    bool32 door_down = false;

    for (u32 screen_index = 0; screen_index < 200; ++screen_index) {

      Assert(random_number_index < ArrayCount(random_number_table));
      u32 random_choice;
      { random_choice = random_number_table[random_number_index++] % 2; }

      bool32 created_z_door = false;
      if (random_choice == 2) {
        created_z_door = true;
        if (abs_tile_z == screen_base_z) {
          door_up = true;
        } else {
          door_down = true;
        }
      } else if (random_choice == 1) {
        door_right = true;
      } else {
        door_top = true;
      }

      for (u32 tile_y = 0; tile_y < tiles_per_height; ++tile_y) {

        for (u32 tile_x = 0; tile_x < tiles_per_width; ++tile_x) {

          u32 abs_tile_x = screen_x * tiles_per_width + tile_x;
          u32 abs_tile_y = screen_y * tiles_per_height + tile_y;

          uint32 tile_value = 1;
          if ((tile_x == 0) &&
              (!door_left || (tile_y != (tiles_per_height / 2)))) {
            tile_value = 2;
          }
          if ((tile_x == (tiles_per_width - 1)) &&
              (!door_right || (tile_y != (tiles_per_height / 2)))) {
            tile_value = 2;
          }
          if ((tile_y == 0) &&
              (!door_down || (tile_x != (tiles_per_width / 2)))) {
            tile_value = 2;
          }
          if ((tile_y == (tiles_per_height - 1)) &&
              (!door_top || (tile_x != (tiles_per_width / 2)))) {
            tile_value = 2;
          }

          if ((tile_x == 10) && (tile_y == 6)) {
            if (door_up) {
              tile_value = 3;
            }

            if (door_down) {
              tile_value = 4;
            }
          }

          if (tile_value == 2) {
            addWall(game_state, abs_tile_x, abs_tile_y, abs_tile_z);
          }
        }
      }

      door_left = door_right;
      door_bottom = door_top;

      if (created_z_door) {
        door_down = !door_down;
        door_up = !door_up;
      } else {
        door_up = false;
        door_down = false;
      }

      door_right = false;
      door_top = false;

      if (random_choice == 2) {
        if (abs_tile_z == screen_base_z) {
          abs_tile_z = screen_base_z + 1;
        } else {
          abs_tile_z = screen_base_z;
        }
      } else if (random_choice == 1) {
        screen_x += 1;
      } else {
        screen_y += 1;
      }
    }

    World_Position new_camera_p = {};
    u32 camera_tile_x = 19 / 2 + screen_base_x * tiles_per_width;
    u32 camera_tile_y = 9 / 2 + screen_base_y * tiles_per_height;
    u32 camera_tile_z = screen_base_z;
    new_camera_p = chunkPositionFromTilePosition(world, camera_tile_x,
                                                 camera_tile_y, camera_tile_z);
    game_state->camera_pos = new_camera_p;

    // TODO(casey): This may be more appropriate to do in the platform layer
    memory->is_initialized = true;
  }

  World* world = game_state->world;

  //
  //
  //  Control updates
  //
  //

  for (int controller_index = 0;
       controller_index < ArrayCount(input->controllers); ++controller_index) {
    Game_Controller_Input* controller = getController(input, controller_index);
    Controlled_Hero* controlled_hero =
        game_state->controlled_heroes + controller_index;
    if (controlled_hero->entity_index == 0) {
      if (controller->start.ended_down) {
        *controlled_hero = {};
        controlled_hero->entity_index =
            createPlayerEntity(game_state).low_index;
      }
    } else {
      controlled_hero->dz = 0.0f;
      controlled_hero->ddp = {};

      if (controller->is_analog) {
        controlled_hero->ddp = v2{controller->left_stick_average_x,
                                  -controller->left_stick_average_y};
      } else {
        // NOTE(casey): Use digital movement tuning
        if (controller->move_up.ended_down) {
          controlled_hero->ddp.y = 1.0f;
        }
        if (controller->move_down.ended_down) {
          controlled_hero->ddp.y = -1.0f;
        }
        if (controller->move_left.ended_down) {
          controlled_hero->ddp.x = -1.0f;
        }
        if (controller->move_right.ended_down) {
          controlled_hero->ddp.x = 1.0f;
        }
      }
    }
  }

  //
  //
  //  Render
  //
  //
  // THis is the old tilemap system
#if 0
#endif

  v2 zero_vect = {0, 0};
  drawRectangle(buffer, zero_vect,
                v2{(real32)buffer->width, (real32)buffer->height}, 0.2f, 0.2f,
                0.2f);
  // drawBitmap(buffer, &game_state->backdrop, 0, 0);

  real32 screen_center_x = 0.5f * (real32)buffer->width;
  real32 screen_center_y = 0.5f * (real32)buffer->height;

  u32 tile_span_x = 25 * 3;
  u32 tile_span_y = 15 * 3;

  Rectangle2 camera_bounds =
      rectCenterDim(V2(0, 0), world->tile_side_in_meters *
                                  V2((real32)tile_span_x, (real32)tile_span_y));
  setCamera(game_state);

  Memory_Arena sim_arena;
  initializeArena(&sim_arena, memory->transient_storage_size,
                  memory->transient_storage);
  Sim_Region* sim_region = beginSim(game_state, &sim_arena, game_state->world,
                                    game_state->camera_pos, camera_bounds);

  Entity_Visible_Piece_Group render_piece_group;
  render_piece_group.game_state = game_state;
  Sim_Entity* entity = sim_region->entities;

  for (u32 entity_index = 0; entity_index < sim_region->entity_count;
       ++entity_index, ++entity) {

    render_piece_group.piece_count = 0;
    real32 dt = input->dt_for_frame;

    real32 player_red = 1.0f;
    real32 player_green = 1.0f;
    real32 player_blue = .0f;

    real32 wall_red = 0.0f;
    real32 wall_green = 1.0f;
    real32 wall_blue = 1.0f;
    real32 meters_to_pixels =
        (real32)world->tile_side_in_pixels / world->tile_side_in_meters;

    Move_Spec move_spec = defaultMoveSpec();
    v2 ddp = {};

    switch (entity->type) {
    case entityType_Hero: {
      for (u32 controlled_index = 0;
           controlled_index < ArrayCount(game_state->controlled_heroes);
           ++controlled_index) {
        Controlled_Hero* controlled_hero =
            game_state->controlled_heroes + controlled_index;

        if (entity->storage_index == controlled_hero->entity_index) {
          entity->dZ = controlled_hero->dz;

          move_spec.unit_max_accel_vector = true;
          move_spec.drag = 8.0f;
          move_spec.speed = 50.0f;
          ddp = controlled_hero->ddp;
        }
        // TODO: Debug stuff remove soon2.
        // RenderPlayer(buffer, input->mouse_x, input->mouse_y);
        // pushRect(&render_piece_group, V2(0,0), 0, V2(entity->width,
        // entity->height), V4(0.0, 1.0, 0.0, 0.0));
        pushBitmap(&render_piece_group, &game_state->DEBUG_bitmap,
                   V2(screen_center_x, screen_center_y), 0.0, V2(0, 0));
      }
      pushRect(&render_piece_group, V2(0, 0), 0,
               V2(entity->width, entity->height),
               V4(player_red, player_green, player_blue, 0.0));
      break;
    }
    case entityType_Wall: {
      pushRect(&render_piece_group, V2(0, 0), 0,
               V2(entity->width, entity->height),
               V4(wall_red, wall_green, wall_blue, 0.0));
      break;
    }
    case entityType_Null: {
      InvalidCodePath;
      break;
    }
    default: {
      InvalidCodePath;
      break;
    }
    }

    if (!isSet(entity, Entity_Flag_Nonspatial)) {
      moveEntity(sim_region, entity, input->dt_for_frame, &move_spec, ddp);
    }

    // Render Time
    real32 entity_ground_point_x =
        screen_center_x + (game_state->meters_to_pixels * entity->p.x);
    real32 entity_ground_point_y =
        screen_center_y - (game_state->meters_to_pixels * entity->p.y);
    real32 entity_z = -game_state->meters_to_pixels * entity->z;

    for (u32 piece_index = 0; piece_index < render_piece_group.piece_count;
         ++piece_index) {
      Entity_Visible_Piece* render_piece =
          render_piece_group.pieces + piece_index;
      v2 center = {entity_ground_point_x + render_piece->offset.x,
                   entity_ground_point_y + render_piece->offset.y +
                       render_piece->offset_z * entity_z};

      if (render_piece->bitmap) {
        drawBitmap(buffer, render_piece->bitmap, center.x, center.y,
                   render_piece->a);
      } else {
        v2 half_dim = 0.5f * game_state->meters_to_pixels * render_piece->dim;
        drawRectangle(buffer, center - half_dim, center + half_dim,
                      render_piece->r, render_piece->g, render_piece->b);
      }
    }
  }
  World_Position world_origin = {};
  World_Difference world_difference =
      worldSubtract(sim_region->world, &world_origin, &sim_region->origin);

  drawRectangle(buffer, world_difference.dXY, V2(10.0, 10.0f), 1.0f, 1.0f,
                0.0f);
  endSim(game_state, sim_region);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples) {
  Game_State* game_state = (Game_State*)memory->permanent_storage;
  gameOutputSound(game_state, sound_buffer, game_state->tone_hz);
}

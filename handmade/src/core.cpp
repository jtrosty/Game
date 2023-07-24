#define ERROR_REMOVER 1

#include "core.h"
#include "core_tilemap.cpp"
#include "core_random.h"
// Bug with offset of pixel rel_colors, everythign got darker.
//

internal void gameOutputSound(Game_State* game_state, Game_Sound_Output_Buffer* sound_buffer, int tone_hz)
{
    i16 tone_volume = 1000;
    int wave_period = sound_buffer->samples_per_second/tone_hz;

    i16* sample_out = sound_buffer->samples;
    for(int sample_index = 0;
        sample_index < sound_buffer->sample_count;
        ++sample_index)
    {
        // TODO(casey): Draw this out for people
#if 1
        real32 sine_value = sinf(game_state->t_sine);
        i16 sample_value = (i16)(sine_value * tone_volume);
#else
        int16 SampleValue = 0;
#endif
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;

        game_state->t_sine += 2.0f*Pi32*1.0f/(real32)wave_period;
        if(game_state->t_sine > 2.0f*Pi32)
        {
            game_state->t_sine -= 2.0f*Pi32;
        }
    }
}

internal void
renderWeirdGradient(Game_Offscreen_Buffer* buffer, int blue_offset, int green_offset)
{
    // TODO(casey): Let's see what the optimizer does

    uint8 *rel_row = (uint8*)buffer->memory;    
    for(int y = 0;
        y < buffer->height;
        ++y)
    {
        uint32* pixel = (uint32*)rel_row;
        for(int x = 0;
            x < buffer->width;
            ++x)
        {
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
};
#pragma pack(pop)


static Loaded_Bitmap DEBUG_loadBMP(debug_platform_read_entire_file* read_entire_file, Thread_Context* thread,  char* filename) {
    // BB GG RR AA, bottom up, start with bottomr row first
    Loaded_Bitmap result = {};
    Debug_Read_File_Result read_result = read_entire_file(thread, filename);
    if (read_result.contents_size != 0) {
        Bitmap_Header* header = (Bitmap_Header*)read_result.contents;
        result.pixels = (u32*)((u8*)read_result.contents + header->bitmap_offset);
        result.width = header->width;
        result.height = header->height;
    }
    else {
        // TODO: Failed ot load
    }
    return result;
}

static void drawBitmap(Game_Offscreen_Buffer* buffer, Loaded_Bitmap* bitmap, 
                        real32 real_x, real32 real_y, i32 align_x, i32 align_y) {
    real_x -= (real32)align_x;
    real_y -= (real32)align_y;

    i32 min_x = roundReal32ToInt32(real_x - (real32)bitmap->width);
    i32 max_x = roundReal32ToInt32(real_x);
    i32 min_y = roundReal32ToInt32(real_y);
    i32 max_y = roundReal32ToInt32(real_y + (real32)bitmap->height);

    i32 source_offset_x = 0;
    if(min_x < 0) {
        source_offset_x = -min_x;
        min_x = 0;
    }
    i32 source_offset_y = 0;
    if (min_y < 0) {
        source_offset_y = -min_y;
        min_y = 0;
    }
    if(max_x > buffer->width) {
        source_offset_x = max_x;
        max_x = buffer->width;
    }
    if(max_y > buffer->height) {
        source_offset_y = max_y;
        max_y = buffer->height;
    }
    u32* source_row = bitmap->pixels + bitmap->width * (bitmap->height - 1);
    source_row += (-source_offset_y * bitmap->width) + source_offset_x;

    u8* dest_row = ((u8*)buffer->memory + max_x * buffer->bytes_per_pixel + max_y * buffer->pitch);

    for (int y = min_y; y < max_y; ++y) {
        u32* dest = (u32*)dest_row;
        u32* source = source_row;
        for (int x = min_x; x < max_x; ++x) {
            real32 A = (real32)((*source >> 24) & 0xFF) / 255.0f;
            real32 SR = (real32)((*source >> 16) & 0xFF);
            real32 SG = (real32)((*source >> 8) & 0xFF);
            real32 SB = (real32)((*source >> 0) & 0xFF);

            real32 DR = (real32)((*dest >> 16) & 0xFF);
            real32 DG = (real32)((*dest >> 8) & 0xFF);
            real32 DB = (real32)((*dest >> 0) & 0xFF);

            real32 R = (1.0f - A) * DR + (A * SR);
            real32 G = (1.0f - A) * DG + (A * SG);
            real32 B = (1.0f - A) * DB + (A * SB);

            *dest = (((uint32)(R + 0.5f) << 16) |
                     ((uint32)(G + 0.5f) << 8) |
                     ((uint32)(B + 0.5f) << 0));
            ++dest;
            ++source;
        }
        dest_row -= buffer->pitch;
        source_row -= bitmap->width;
    }
}

static void drawRectangle(Game_Offscreen_Buffer* buffer, 
                            v2 v_min, v2 v_max,
                            real32 r, real32 g, real32 b) {

    i32 min_x = roundReal32ToInt32(v_min.x);
    i32 min_y = roundReal32ToInt32(v_min.y);
    i32 max_x = roundReal32ToInt32(v_max.x);
    i32 max_y = roundReal32ToInt32(v_max.y);

    if (min_x < 0) {
        min_x = 0;
    }
    if(min_y < 0) {
        min_y = 0;
    }
    if (max_x > buffer->width) {
        max_x = buffer->width;
    }
    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    u32 rel_color = ((roundReal32ToUint32(r * 255.0f) << 16) |
                 (roundReal32ToUint32(g * 255.0f) << 8)  |
                 (roundReal32ToUint32(b * 255.0f) << 0));

    u8* rel_row = ((u8*)buffer->memory +  min_x * buffer->bytes_per_pixel + min_y * buffer->pitch);
    for (int y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)rel_row;
        for (int x = min_x; x < max_x; ++x) {
            *pixel++ = rel_color;
        }
        rel_row += buffer->pitch;
    }
}

static void renderPlayer(Game_Offscreen_Buffer* buffer, int player_x, int player_y, int length_of_side) {
    u32 rel_color = 0xFF00ff00; //FFFFFF;
}

inline Entity* getEntity(Game_State* game_state, u32 index) {
    Entity* entitiy = 0;

    if((index > 0) && (index < ArrayCount(game_state->entities))) {
        entitiy = &game_state->entities[index];
    }
    return entitiy;
}

static u32 addEntity(Game_State* game_state) {
    u32 entity_index = game_state->entity_count++;
    Assert(game_state->entity_count < ArrayCount(game_state->entities));
    Entity* entity = &game_state->entities[entity_index];
    *entity = {};

    return entity_index;
}

static bool32 testWall (real32 wall_x, 
                        real32 rel_x, real32 rel_y, 
                        real32 player_delta_x, real32 player_delta_y,
                        real32* t_min, real32 min_y, real32 max_y) {
    bool32 hit = false;

    real32 t_epsilon = 0.0001f;
    // We make sure there is movement
    if(player_delta_x != 0.0f) {
        real32 t_result = (wall_x - rel_x) / player_delta_x;
        real32 y = rel_y + t_result * player_delta_y;
        if((t_result >= 0.0f) && (*t_min > t_result)) {
            if ((y >= min_y) && (y <= max_y)) {

                *t_min = Maximum(0.0f, t_result - t_epsilon);
                hit = true;
            }
        }
    }
    return hit;
}

static void initialize_player(Game_State* game_state, u32 entity_index) {
    Entity* entity = getEntity(game_state, entity_index);
    
    entity->exists = true;
    entity->p.abs_tile_x = 17 / 2 + 5;
    entity->p.abs_tile_y = 9 / 2;
    entity->p.abs_tile_z = 0;
    entity->p.offset.x = 0;
    entity->p.offset.y = 0;
    entity->heigth = 2.0f;
    entity->width = 2.0f;

    if (!getEntity(game_state, game_state->camera_following_entity_index)) {
        game_state->camera_following_entity_index = entity_index;
    }
}

internal void movePlayer(Game_State* game_state, Entity* entity, real32 dt, v2 ddp) {

    Tile_Map* tile_map = game_state->world->tile_map;

    real32 ddp_length = math_lenghtSq(ddp);
    if(ddp_length > 1.0f) {
        ddp *= (1.0f / squareRoot(ddp_length));
    }

    real32 player_speed = 50.0f;
    real32 player_friciton = -8.0f;

    // ODEs here
    ddp *= player_speed;
    ddp += player_friciton * entity->dp;

    Tile_Map_Position old_player_pos = entity->p;
    // p' = 1/2 a t^32 + vt + p;
    v2 player_delta = (0.5f * ddp * math_square(dt) +
                       entity->dp * dt);
    entity->dp = ddp * dt + entity->dp;
    Tile_Map_Position new_player_pos = offsetTilePosition(tile_map, old_player_pos, player_delta);

    // TODO: ODE here?
    //new_player_pos = recanonicalizePosition(tile_map, new_player_pos);
    u32 min_tile_x = Minimum(old_player_pos.abs_tile_x, new_player_pos.abs_tile_x);
    u32 min_tile_y = Minimum(old_player_pos.abs_tile_y, new_player_pos.abs_tile_y);
    u32 max_tile_x = Maximum(old_player_pos.abs_tile_x, new_player_pos.abs_tile_x);
    u32 max_tile_y = Maximum(old_player_pos.abs_tile_y, new_player_pos.abs_tile_y);
    
    u32 entity_tile_width = ceilReal32ToInt32(entity->width / tile_map->tile_side_in_meters);
    u32 entity_tile_height = ceilReal32ToInt32(entity->heigth / tile_map->tile_side_in_meters);

    min_tile_x -= entity_tile_width;
    min_tile_y -= entity_tile_height;
    max_tile_x += entity_tile_width;
    max_tile_y += entity_tile_height;

    u32 abs_tile_z = entity->p.abs_tile_z;

    real32 t_remaining = 1.0f;
    for (u32 iteration = 0; (iteration < 4) && (t_remaining > 0.0f); ++iteration) {

        real32 t_min = 1.0f;
        v2 wall_normal = {};

        Assert((max_tile_x - min_tile_x) < 32);
        Assert((max_tile_y - min_tile_y) < 32);

        for (u32 abs_tile_y = min_tile_y; abs_tile_y <= max_tile_y; ++abs_tile_y) {
            for (u32 abs_tile_x = min_tile_x; abs_tile_x <= max_tile_x; ++abs_tile_x) {

                Tile_Map_Position test_tile_p = centeredTilePoint(abs_tile_x, abs_tile_y, abs_tile_z);
                u32 tile_value = getTileValue(tile_map, test_tile_p);

                if (!isTileValueEmpty(tile_value)) {

                    real32 diameter_w = tile_map->tile_side_in_meters + entity->width;
                    real32 diameter_h = tile_map->tile_side_in_meters + entity->heigth;

                    v2 min_corner = -0.5f * v2{diameter_w, diameter_h};
                    v2 max_corner =  0.5f * v2{diameter_w, diameter_h};

                    Tile_Map_Difference rel_old_player_p = tileSubtract(tile_map, &entity->p, &test_tile_p);
                    v2 rel = rel_old_player_p.dXY;

                    if (testWall(min_corner.x, 
                                 rel.x, rel.y, 
                                 player_delta.x, player_delta.y, 
                                 &t_min, min_corner.y, max_corner.y)) {
                        wall_normal = v2{-1, 0};
                    }
                    if (testWall(max_corner.x, rel.x, rel.y, player_delta.x, player_delta.y, &t_min, min_corner.y, max_corner.y)) {

                        wall_normal = v2{1, 0};
                    }
                    if (testWall(min_corner.y, rel.x, rel.y, player_delta.y, player_delta.x, &t_min, min_corner.x, max_corner.x)) {

                        wall_normal = v2{0, -1};
                    }
                    if (testWall(max_corner.y, rel.x, rel.y, player_delta.y, player_delta.x, &t_min, min_corner.x, max_corner.x)) {

                        wall_normal = v2{0, 1};
                    }
                }
            }
        }
        entity->p = offsetTilePosition(tile_map, entity->p, t_min * player_delta);
        entity->dp = entity->dp + 1 * math_inner(entity->dp, wall_normal) * wall_normal;
        player_delta = player_delta + 1 * math_inner(player_delta, wall_normal) * wall_normal;
        t_remaining -= t_min * t_remaining;
    }


    // NOTE: Update camera/player Z based on last movement
    if (!areOnSameTile(&old_player_pos, &entity->p)) {
        u32 new_tile_value = getTileValue(tile_map, entity->p);
        
        if(new_tile_value == 3) {
            ++entity->p.abs_tile_z;
        }
        else if(new_tile_value == 4) {
            --entity->p.abs_tile_z;
        }
    }
    if ((entity->dp.x == 0.0f) && (entity->dp.y == 0.0f)) {

    }
    else if (absoluteValue(entity->dp.x) > absoluteValue(entity->dp.y)) {
        // Code here to test facing direciton, not of interest to me right now.
    }
}

static void initializeArena(Memory_Arena* arena, memory_index size, u8* base) {
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

#define pushStruct(arena, type) (type*)pushSize_(arena, sizeof(type))
#define pushArray(arena, count, type) (type*)pushSize_(arena, (count) * sizeof(type))
static void* pushSize_(Memory_Arena* arena, memory_index size) {
    // All memory when initial gained from valloc is cleared to zero
    Assert((arena->used + size) <= arena->size);
    void* result = arena->base + arena->used;
    arena->used += size;

    return result;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    // name(Thread_Context* thread, Game_Memory* memory, Game_Input* input, Game_Offscreen_Buffer* buffer)

    Assert(sizeof(Game_State) <= memory->permanent_storage_size);
    Assert((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
        (ArrayCount(input->controllers[0].buttons)));


    Game_State* game_state = (Game_State*)memory->permanent_storage;

    if(!memory->is_initialized) {
        // TODO: Remove?

        char* filename = __FILE__;

        addEntity(game_state);

        game_state->DEBUG_bitmap = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/deub_bitmap.bmp");

        Hero_Bitmaps* hero_bitmaps;
        hero_bitmaps = game_state->hero_bitmaps;
        hero_bitmaps[0].head = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_right_head.bmp");
        hero_bitmaps[0].cape = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_right_cape.bmp");
        hero_bitmaps[0].torso = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_right_torso.bmp");
        hero_bitmaps[0].align_x = 72;
        hero_bitmaps[0].align_y = 182;

        hero_bitmaps[1].head = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_back_head.bmp");
        hero_bitmaps[1].cape = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_back_cape.bmp");
        hero_bitmaps[1].torso = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_back_torso.bmp");
        hero_bitmaps[1].align_x = 72;
        hero_bitmaps[1].align_y = 182;

        hero_bitmaps[2].head =  DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_left_head.bmp");
        hero_bitmaps[2].cape =  DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_left_cape.bmp");
        hero_bitmaps[2].torso = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_left_torso.bmp");
        hero_bitmaps[2].align_x = 72;
        hero_bitmaps[2].align_y = 182;

        hero_bitmaps[3].head =  DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_front_head.bmp");
        hero_bitmaps[3].cape =  DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_front_cape.bmp");
        hero_bitmaps[3].torso = DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_hero_front_torso.bmp");
        hero_bitmaps[3].align_x = 72;
        hero_bitmaps[3].align_y = 182;

        game_state->camera_pos.abs_tile_x = 17 / 2;
        game_state->camera_pos.abs_tile_y = 9 / 2;

        
        /*
        Debug_Read_File_Result file = memory->DEBUG_platformReadEntireFile(thread, filename);
        if(file.contents)
        {
            memory->DEBUG_platformWriteEntireFile(thread, "test.out", file.contents_size, file.contents);
            memory->DEBUG_platformFreeFileMemory(thread, file.contents);
        }
        */

        initializeArena(&game_state->world_arena, 
                        memory->permanent_storage_size - sizeof(Game_State), 
                        (u8*)memory->permanent_storage + sizeof(Game_State));
        game_state->world = pushStruct(&game_state->world_arena, World);
        World* world = game_state->world; 

        world->tile_map = pushStruct(&game_state->world_arena, Tile_Map);
        Tile_Map* tile_map = world->tile_map;

        // This is set to 256x256 tile chunks.
        tile_map->chunk_shift = 4;
        tile_map->chunk_mask = (1 << tile_map->chunk_shift) - 1;
        tile_map->chunk_dim = (1 << tile_map->chunk_shift);

        tile_map->tile_chunk_count_x = 128;
        tile_map->tile_chunk_count_y = 128;
        tile_map->tile_chunk_count_z = 1;
        tile_map->tile_chunks = pushArray(&game_state->world_arena,
                                          (tile_map->tile_chunk_count_x * tile_map->tile_chunk_count_y),
                                          Tile_Chunk);

        for (u32 y = 0; y < tile_map->tile_chunk_count_y; ++y) {
            for (u32 x = 0; x < tile_map->tile_chunk_count_x; ++x) {
                tile_map->tile_chunks[y * tile_map->tile_chunk_count_x + x].tiles = 
                    pushArray(&game_state->world_arena, tile_map->chunk_dim * tile_map->chunk_dim, u32);
            }
        }

        //tile_map->tile_chunks = &tile_chunk;
        tile_map->tile_side_in_meters = 1.0f;
        tile_map->tile_side_in_pixels = 10;
        tile_map->meters_to_pixels = (real32)tile_map->tile_side_in_pixels / 
                                        (real32)tile_map->tile_side_in_meters;
        game_state->tone_hz = 512;
        game_state->player_speed_scaler = 10.0f;

        u32 lower_left_x = (real32)tile_map->tile_side_in_pixels / 2;
        u32 lower_left_y = (real32)buffer->height;

        u32 tiles_per_width = 17;
        u32 tiles_per_height = 9;
        u32 abs_tile_z = 0;
        for (u32 screen_y = 0; screen_y < 32; ++screen_y) {
            for (u32 screen_x = 0; screen_x < 32; ++screen_x){ 
                for (u32 tile_y = 0; tile_y < tiles_per_height; tile_y++) {
                    for (u32 tile_x = 0; tile_x < tiles_per_width; tile_x++) {
                        u32 abs_tile_x = screen_x * tiles_per_width + tile_x;
                        u32 abs_tile_y = screen_y * tiles_per_height + tile_y;

                        u32 tile_value = 0;
                        // TODO: Not used remove?
                        if ((tile_x == tile_y) && (tile_y % 2))
                            tile_value = 2;
                        else 
                            tile_value = 1;
                        if (tile_y == 2) {
                            tile_value = 2;
                        }

                        setTileValue(&game_state->world_arena, tile_map, abs_tile_x, abs_tile_y, abs_tile_z, 
                                      tile_value);
                    }
                }
            }
        }

        // TODO(casey): This may be more appropriate to do in the platform layer
        memory->is_initialized = true;
    }
    World* world = game_state->world; 
    Tile_Map* tile_map = world->tile_map;

    //
    //
    //  Control updates
    //
    //
    game_state->player_speed_scaler = 1000.0f;

    for(int controller_index = 0;
        controller_index < ArrayCount(input->controllers);
        ++controller_index)
    {
        Game_Controller_Input* controller = getController(input, controller_index);
        Entity* controlling_entity = getEntity(game_state, game_state->player_index_for_controllers[controller_index]);
        if(controlling_entity) {
            v2 ddp = {};
            if(controller->is_analog) {
                ddp = v2{controller->left_stick_average_x, -controller->left_stick_average_y};
            }
            else {
                // NOTE(casey): Use digital movement tuning
                if(controller->move_up.ended_down) {
                    ddp.y = 1.0f;
                }
                if(controller->move_down.ended_down) {
                    ddp.y = -1.0f;
                }
                if(controller->move_left.ended_down) {
                    ddp.x = -1.0f;
                }
                if(controller->move_right.ended_down) {
                    ddp.x = 1.0f;
                }
            }
            movePlayer(game_state, controlling_entity, input->dt_for_frame, ddp);
            if (ddp.x > 1000 || ddp.y > 1000) {
                movePlayer(game_state, controlling_entity, input->dt_for_frame, ddp);
            }
        }
        else {
            if (controller->start.ended_down) {
                u32 entity_index = addEntity(game_state);
                initialize_player(game_state, entity_index);
                game_state->player_index_for_controllers[controller_index] = entity_index;
            }
        }
    }

    Entity* camera_following_entity = getEntity(game_state, game_state->camera_following_entity_index);
    if (camera_following_entity) {
        game_state->camera_pos.abs_tile_z = camera_following_entity->p.abs_tile_z;

        Tile_Map_Difference diff = tileSubtract(tile_map, &camera_following_entity->p, &game_state->camera_pos);
        real32 distance_from_center = 3.0f * tile_map->tile_side_in_meters;
        Entity* entity_camera_follows = &(game_state->entities[game_state->camera_following_entity_index]);

        if(diff.dXY.x > (distance_from_center)) {
            game_state->camera_pos.abs_tile_x = entity_camera_follows->p.abs_tile_x - distance_from_center;
            game_state->camera_pos.offset.x = entity_camera_follows->p.offset.x;
        }
        if(diff.dXY.x < -(distance_from_center)) {
            game_state->camera_pos.abs_tile_x = entity_camera_follows->p.abs_tile_x + distance_from_center;
            game_state->camera_pos.offset.x = entity_camera_follows->p.offset.x;
        }
        if(diff.dXY.y > (distance_from_center)) {
            game_state->camera_pos.abs_tile_y = entity_camera_follows->p.abs_tile_y - distance_from_center;
            game_state->camera_pos.offset.y = entity_camera_follows->p.offset.y;
        }
        if(diff.dXY.y < -(distance_from_center)) {
            game_state->camera_pos.abs_tile_y = entity_camera_follows->p.abs_tile_y + distance_from_center;
            game_state->camera_pos.offset.y = entity_camera_follows->p.offset.y;
        }
    }

    v2 zero_vect;
    zero_vect.x = 0;
    zero_vect.y = 0;
    v2 buffer_side;
    buffer_side.x = (real32)buffer->width;
    buffer_side.y = (real32)buffer->height;


    //
    //
    //  Render
    //
    //



    drawRectangle(buffer, zero_vect, buffer_side, 0.0f, 0.0f, 0.0f);

    real32 screen_center_x = 0.5f * (real32)buffer->width;
    real32 screen_center_y = 0.5f * (real32)buffer->height;

    int test_gradient = 0;
    for (i32 rel_row = -100; rel_row < 100; ++rel_row) {
        for (i32 rel_col = -200; rel_col < 200; ++rel_col) {
            u32 column = game_state->camera_pos.abs_tile_x + rel_col;
            u32 row = game_state->camera_pos.abs_tile_y + rel_row;
            u32 z = 0;
            u32 tile_id = getTileValue(tile_map, column, row, z);
            // Empty
            // 1, 3, 4

            real32 gray = 0.4f;
            if ( tile_id > 0) {
                if (tile_id == 2) {
                    gray = 0.7f;
                }
                if (tile_id == 1) {
                    gray = 0.4f;
                }
                //gray *= ((real32)(test_gradient) / (real32)(17 * 9));
                if ((column == game_state->camera_pos.abs_tile_x) && 
                    (row == game_state->camera_pos.abs_tile_y)) {
                    gray = 0.0f;
                }

                v2 cen = { screen_center_x - (tile_map->meters_to_pixels * game_state->camera_pos.offset.x) + 
                           ((real32)rel_col) * tile_map->tile_side_in_pixels,  
                           screen_center_y + (tile_map->meters_to_pixels * game_state->camera_pos.offset.y) - 
                           ((real32)rel_row) * tile_map->tile_side_in_pixels
                        };

                v2 min = { cen.x - 0.5f * tile_map->tile_side_in_pixels, cen.y - 0.5f * tile_map->tile_side_in_pixels};
                v2 max = { cen.x + 0.5f * tile_map->tile_side_in_pixels, cen.y + 0.5f * tile_map->tile_side_in_pixels};

                drawRectangle(buffer, min, max, gray, gray, gray); }
            //test_gradient++;
        }
    }

    Entity* entity_ptr = game_state->entities;
    for (u32 entity_index = 0; 
         entity_index < game_state->entity_count; 
         ++entity_index, ++entity_ptr) {
        if(entity_ptr->exists) {
            Tile_Map_Difference diff = tileSubtract(tile_map, &entity_ptr->p, &game_state->camera_pos);
            real32 player_ground_point_x = screen_center_x + tile_map->meters_to_pixels * diff.dXY.x; 
            real32 player_ground_point_y = screen_center_y - tile_map->meters_to_pixels * diff.dXY.y; 
            v2 player_left_bottom = { player_ground_point_x - (real32)(0.5f * tile_map->meters_to_pixels * entity_ptr->width), 
                                      player_ground_point_y - (real32)(tile_map->meters_to_pixels * entity_ptr->heigth)};
            v2 length_player = {(real32)entity_ptr->width, (real32)entity_ptr->heigth};

            // Turn player pos into a pixel locaiton on the screen.

            Hero_Bitmaps* hero_bitmaps = &game_state->hero_bitmaps[entity_ptr->facingDirection];
            //drawBitmap(buffer, &hero_bitmaps->torso, player_ground_point_x, player_ground_point_y, hero_bitmaps->align_x, hero_bitmaps->align_y);
            //drawBitmap(buffer, &hero_bitmaps->cape, player_ground_point_x, player_ground_point_y, hero_bitmaps->align_x, hero_bitmaps->align_y);
            //drawBitmap(buffer, &hero_bitmaps->head, player_ground_point_x, player_ground_point_y, hero_bitmaps->align_x, hero_bitmaps->align_y);

            real32 player_red = 0.0f;
            real32 player_green = 1.0f;
            real32 player_blue = 1.0f;
            u32 rel_color = 0xFF00ff00; //FFFFFF;
            drawRectangle(buffer, player_left_bottom, player_left_bottom + (length_player * 5),
                          player_red, player_green, player_blue);
        }
    }


    //RenderPlayer(buffer, input->mouse_x, input->mouse_y);
    // TODO: Debug stuff remove soon.
    drawRectangle(buffer, zero_vect, {40.0, 40.0}, 0.0f, 1.0f, 0.0f);
    drawBitmap(buffer, &game_state->DEBUG_bitmap, screen_center_x, screen_center_y, 0, 0);

    for(int button_index = 0;
        button_index < ArrayCount(input->mouse_buttons);
        ++button_index)
    {
        if(input->mouse_buttons[button_index].ended_down)
        {
            //RenderPlayer(Buffer, 10 + 20*ButtonIndex, 10);
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    Game_State* game_state = (Game_State*)memory->permanent_storage;
    gameOutputSound(game_state, sound_buffer, game_state->tone_hz);
}

/* bool32 is_valid = false;
        if ((player_tile_x >= 0) && (player_tile_x < TILE_MAP_COUNT_X) &&
            (player_tile_y >= 0) && (player_tile_y < TILE_MAP_COUNT_Y)) {
                u32 tile_map_value = active_tile_map->tiles[(player_tile_y * TILE_MAP_COUNT_X) + player_tile_x];
                is_valid = (tile_map_value == 1);
                if (tile_map_value == 1) {
                    is_valid = 1;
                }
                else {
                    is_valid = 0;
                }
        }
        if (is_valid) {
            game_state->player_x = new_player_x;
            game_state->player_y = new_player_y;
            game_state->player_tile_x = player_tile_x;
            game_state->player_tile_y = player_tile_y;
        }
*/

        //game_state->player_x += truncateReal32ToInt32(move_scale * (input->dt_for_frame) * (controller->left_stick_average_x));
        //game_state->player_y += truncateReal32ToInt32(move_scale * (input->dt_for_frame) * (controller->left_stick_average_y));

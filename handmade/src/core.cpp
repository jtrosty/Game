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


static u32* DEBUG_loadBMP(debug_platform_read_entire_file* read_entire_file, Thread_Context* thread,  char* filename) {

    u32* result = 0;
    Debug_Read_File_Result read_result = read_entire_file(thread, filename);
    if (read_result.contents_size != 0) {
        Bitmap_Header* header = (Bitmap_Header*)read_result.contents;
        u32* pixels = (u32*)((u8*)read_result.contents + header->bitmap_offset);
        result = pixels;
    }
    else {
        // TODO: Failed ot load
    }
    return result;
}

static void drawRectangle(Game_Offscreen_Buffer* buffer, 
                            real32 real_min_x, real32 real_min_y, real32 real_max_x, real32 real_max_y,
                            real32 r, real32 g, real32 b) {

    i32 min_x = roundReal32ToInt32(real_min_x);
    i32 min_y = roundReal32ToInt32(real_min_y);
    i32 max_x = roundReal32ToInt32(real_max_x);
    i32 max_y = roundReal32ToInt32(real_max_y);

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
    drawRectangle(buffer, (real32)player_x, (real32)player_y, 
                        (real32)(player_x + length_of_side),(real32)(player_y + length_of_side), 0.0f, 1.0f, 1.0f);
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
        DEBUG_loadBMP(*memory->DEBUG_platformReadEntireFile, thread, "../test/test_background.bmp");
        char* filename = __FILE__;
        game_state->player_pos.abs_tile_x = 1;
        game_state->player_pos.abs_tile_y = 1;
        game_state->player_pos.offset_x = 0.5f;
        game_state->player_pos.offset_y = 0.5f;
        
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
        tile_map->tile_chunks = pushArray(&game_state->world_arena,
                                          (tile_map->tile_chunk_count_x * tile_map->tile_chunk_count_y),
                                          Tile_Chunk);

        for (i32 y = 0; y < tile_map->tile_chunk_count_y; ++y) {
            for (i32 x = 0; x < tile_map->tile_chunk_count_x; ++x) {
                tile_map->tile_chunks[y * tile_map->tile_chunk_count_x + x].tiles = 
                    pushArray(&game_state->world_arena, tile_map->chunk_dim * tile_map->chunk_dim, u32);
            }
        }


        //tile_map->tile_chunks = &tile_chunk;
        tile_map->tile_side_in_meters = 1.0f;
        tile_map->tile_side_in_pixels = 20;
        tile_map->meters_to_pixels = (real32)tile_map->tile_side_in_pixels / 
                                        (real32)tile_map->tile_side_in_meters;
        

        game_state->player_side_length = tile_map->tile_side_in_meters / 6.0f;
        game_state->player_side_length_pixels = tile_map->tile_side_in_pixels * 
                    (game_state->player_side_length / tile_map->tile_side_in_meters);
        game_state->tone_hz = 512;
        game_state->player_speed_scaler = 10.0f;
        game_state->player_max_distance_per_frame = 1.0f;

    u32 lower_left_x = (real32)tile_map->tile_side_in_pixels / 2;
    u32 lower_left_y = (real32)buffer->height;

        u32 tiles_per_width = 17;
        u32 tiles_per_height = 9;
        for (u32 screen_y = 0; screen_y < 32; ++screen_y) {
            for (u32 screen_x = 0; screen_x < 32; ++screen_x){ 
                for (u32 tile_y = 0; tile_y < tiles_per_height; tile_y++) {
                    for (u32 tile_x = 0; tile_x < tiles_per_width; tile_x++) {
                        u32 abs_tile_x = screen_x * tiles_per_width + tile_x;
                        u32 abs_tile_y = screen_y * tiles_per_height + tile_y;

                        u32 tile_value = 0;
                        // TODO: Not used remove?
                        if ((tile_x == 0) || (tile_x == (tiles_per_width - 1))) {
                            if (tile_y == (tiles_per_height / 2)) {
                                tile_value = 1;
                            }
                            else {
                                tile_value = 0;
                            }
                        }
                        if ((tile_y == 0) || (tile_y == (tiles_per_height - 1))) {
                            if (tile_x == (tiles_per_width / 2)) {
                                tile_value = 1;
                            }
                            else {
                                tile_value = 0;
                            }
                        }

                        setTileValue(&game_state->world_arena, tile_map, abs_tile_x, abs_tile_y, 
                                     (tile_x == tile_y) && (tile_y % 2) ? 2 : 1);
                    }
                }
            }
        }

        // TODO(casey): This may be more appropriate to do in the platform layer
        memory->is_initialized = true;
    }
    World* world = game_state->world; 
    Tile_Map* tile_map = world->tile_map;

    //Tile_Chunk* active_tile_map = getTileChunk(&world-> game_state->player_pos.abs_tile_x, game_state->player_pos.abs_tile_y);
    
    //drawRectangle(buffer, 0.0f, 0.0f, (real32)buffer->width, (real32)buffer->height, 1.0f, 0.0f, 1.0f )
    //renderWeirdGradient(buffer, game_state->blue_offset, game_state->green_offset);

    for(int controller_index = 0;
        controller_index < ArrayCount(input->controllers);
        ++controller_index)
    {
        Game_Controller_Input* controller = getController(input, controller_index);
        if(controller->is_analog) {
            // NOTE(casey): Use analog movement tuning
            game_state->blue_offset += (int)(4.0f*controller->left_stick_average_x);
            game_state->tone_hz = 512 + (int)(128.0f*controller->left_stick_average_y);
        }
        else {
            // NOTE(casey): Use digital movement tuning
            if(controller->move_left.ended_down)
            {
                game_state->blue_offset -= 1;
            }
            
            if(controller->move_right.ended_down)
            {
                game_state->blue_offset += 1;
            }
        }

        // Input.AButtonEndedDown;
        // Input.AButtonHalfTransitionCount;

        Tile_Map_Position new_player_pos =  game_state->player_pos;
        real32 player_x_diff = game_state->player_speed_scaler * input->dt_for_frame * controller->left_stick_average_x;
        real32 player_y_diff = game_state->player_speed_scaler * input->dt_for_frame * controller->left_stick_average_y;

        new_player_pos.offset_x += clampAbsValueReal32(player_x_diff, game_state->player_max_distance_per_frame);
        new_player_pos.offset_y -= clampAbsValueReal32(player_y_diff, game_state->player_max_distance_per_frame);

        new_player_pos = recanonicalizePosition(tile_map, new_player_pos);


        //bool32 test = isworld->ointEmpty(&world, new_player_pos);
        bool32 update_pos = isTileMapPointEmpty(tile_map, &new_player_pos);
        if(!update_pos){
            if (isTileMapPointEmpty(tile_map, &new_player_pos)) {
                game_state->player_pos = new_player_pos;
            }
        }
        if (isTileMapPointEmpty(tile_map, &new_player_pos)) {
            game_state->player_pos = new_player_pos;
        }
    }

    drawRectangle(buffer, 0.0f, 0.0f, (real32)buffer->width, (real32)buffer->height,
                  1.0f, 0.0f, 0.0f);

    real32 screen_center_x = 0.5f * (real32)buffer->width;
    real32 screen_center_y = 0.5f * (real32)buffer->height;

    int test_gradient = 0;
    for (i32 rel_row = -100; rel_row < 100; ++rel_row) {
        for (i32 rel_col = -200; rel_col < 200; ++rel_col) {
            u32 column = game_state->player_pos.abs_tile_x + rel_col;
            u32 row = game_state->player_pos.abs_tile_y + rel_row;
            u32 tile_id = getTileValue(tile_map, column, row);

            real32 gray = 0.4f;
            if ( tile_id > 0) {
                if (tile_id == 2) {
                    gray = 0.7f;
                }
                if (tile_id == 1) {
                    gray = 0.4f;
                }
                //gray *= ((real32)(test_gradient) / (real32)(17 * 9));
                if ((column == game_state->player_pos.abs_tile_x) && 
                    (row == game_state->player_pos.abs_tile_y)) {
                    gray = 0.0f;
                }

                real32 cen_x = screen_center_x - 
                        (tile_map->meters_to_pixels * game_state->player_pos.offset_x) + 
                        ((real32)rel_col) * tile_map->tile_side_in_pixels;
                real32 cen_y = screen_center_y + 
                        (tile_map->meters_to_pixels * game_state->player_pos.offset_y) - 
                        ((real32)rel_row) * tile_map->tile_side_in_pixels;

                real32 min_x = cen_x - 0.5f * tile_map->tile_side_in_pixels;
                real32 min_y = cen_y - 0.5f * tile_map->tile_side_in_pixels;
                real32 max_x = cen_x + 0.5f * tile_map->tile_side_in_pixels;
                real32 max_y = cen_y + 0.5f * tile_map->tile_side_in_pixels;

                drawRectangle(buffer, min_x, min_y, max_x, max_y, gray, gray, gray);
            }

            //test_gradient++;
        }
    }

    // Turn player pos into a pixel locaiton on the screen.
    int player_x_left_pixel = screen_center_x - 
                        (0.5f * tile_map->meters_to_pixels * game_state->player_side_length);
    int player_y_bottom_pixel = screen_center_y - 
                        (0.5f * tile_map->meters_to_pixels * game_state->player_side_length);
    real32 player_red = 0.0f;
    real32 player_green = 1.0f;
    real32 player_blue = 1.0f;
    u32 rel_color = 0xFF00ff00; //FFFFFF;
    drawRectangle(buffer, player_x_left_pixel, player_y_bottom_pixel, 
                        (player_x_left_pixel + game_state->player_side_length_pixels),
                        (player_y_bottom_pixel + game_state->player_side_length_pixels), 
                        player_red, player_green, player_blue);

    //RenderPlayer(buffer, input->mouse_x, input->mouse_y);
    drawRectangle(buffer, 0, 0, 40, 40, 0.0f, 1.0f, 0.0f);

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
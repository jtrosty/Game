#include "core.h"
// Bug with offset of pixel colors, everythign got darker.
//

u32 const TILE_MAP_COUNT_X = 17;
u32 const TILE_MAP_COUNT_Y = 9;

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

    uint8 *row = (uint8*)buffer->memory;    
    for(int y = 0;
        y < buffer->height;
        ++y)
    {
        uint32* pixel = (uint32*)row;
        for(int x = 0;
            x < buffer->width;
            ++x)
        {
            uint8 blue = (uint8)(x + blue_offset);
            uint8 green = (uint8)(y + green_offset);

            *pixel++ = ((green << 16) | blue);
        }
        
        row += buffer->pitch;
    }
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

    u32 color = ((roundReal32ToUint32(r * 255.0f) << 16) |
                 (roundReal32ToUint32(g * 255.0f) << 8)  |
                 (roundReal32ToUint32(b * 255.0f) << 0));

    u8* row = ((u8*)buffer->memory +  min_x * buffer->bytes_per_pixel + min_y * buffer->pitch);
    for (int y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)row;
        for (int x = min_x; x < max_x; ++x) {
            *pixel++ = color;
        }
        row += buffer->pitch;
    }
}

static void renderPlayer(Game_Offscreen_Buffer* buffer, int player_x, int player_y, int length_of_side) {
    u8* end_of_buffer = (u8*)buffer->memory + (buffer->pitch * buffer->height);
    u32 color = 0xFF00ff00; //FFFFFF;
    
    int top = player_y;
    int bottom = player_y + length_of_side;
    for (int x = player_x; x < player_x + length_of_side; ++x) {
        u8* pixel = ((u8*)buffer->memory + (x * buffer->bytes_per_pixel) + (top * buffer->pitch));
        for (int y = top; y < bottom; ++y) {
            if ((pixel >= buffer->memory) && ((pixel + 4) <= end_of_buffer)) {
                *(u32*)pixel = color;
            }
            pixel += buffer->pitch;
        }
    }
    drawRectangle(buffer, (real32)player_x, (real32)player_y, 
                        (real32)(player_x + length_of_side),(real32)(player_y + length_of_side), 0.0f, 1.0f, 1.0f);
}

inline Tile_Map* getTileMap(World* world, i32 tile_map_x, i32 tile_map_y) { 
    Tile_Map* tile_map = 0;
    if ((tile_map_x >= 0) && (tile_map_x < world->tile_map_count_x) &&
        (tile_map_y >= 0) && (tile_map_y < world->tile_map_count_y)) {
            tile_map = &world->tile_map[tile_map_y * world->tile_map_count_x + tile_map_x];
    }
    else {
        if (tile_map_x < 0) {
            tile_map_x = world->tile_map_count_x + tile_map_x;
        }
        else if (tile_map_x > world->tile_map_count_x) {

        }
        if (tile_map_y < 0) {

        }
        else if (tile_map_y > world->tile_map_count_y) {

        }
        tile_map = &world->tile_map[tile_map_y * world->tile_map_count_x + tile_map_x];
    }
    return tile_map;
}

inline u32 getTileValueUnchecked(World* world, Tile_Map* tile_map, i32 tile_x, i32 tile_y) {
    Assert(tile_map);
    Assert((tile_x >= 0) && (tile_x < world->count_x) && (tile_y >= 0) && (tile_y < world->count_y));
    u32 tile_map_value = tile_map->tiles[tile_y * world->count_x + tile_x];
    return tile_map_value;
}

inline bool32 isTileMapPointEmpty(World* world, Tile_Map* tile_map, i32 test_tile_x, i32 test_tile_y) {
    bool32 result = false;
    // This funciton is guarded by checking if tile_map is zero or not.
    if (tile_map) {
        if ((test_tile_x >= 0) && (test_tile_x < world->count_x) &&
            (test_tile_y >= 0) && (test_tile_y < world->count_y)) {
                u32 tile_map_value = getTileValueUnchecked(world, tile_map, test_tile_x, test_tile_y);
                result = (tile_map_value == 1);
            }
    }
    return result;
}

inline void recanonicalizeCoordinate(World* world, i32 tile_map_count, i32 tile_count, i32* tile_map, i32* tile, real32* tile_rel) {
    i32 offset = floorReal32ToInt32(*tile_rel / world->tile_side_in_meters); // this will be 1 or -1
    *tile += offset;
    *tile_rel -= offset * world->tile_side_in_meters;

    Assert(*tile_rel >= 0);
    Assert(*tile_rel <= world->tile_side_in_meters);

    if (*tile < 0) {
        *tile = tile_count + *tile;
        *tile_map -= 1;
        if (*tile_map < 0) {
            *tile_map += world->tile_map_count_x;
        }
    }
    if (*tile >= tile_count) {
        *tile = tile_count - *tile;
        *tile_map += 1;
        if (*tile_map >= tile_map_count) {
            *tile_map -= tile_map_count;
        }
    }
}

inline World_Position recanonicalizePosition(World* world, World_Position pos) {
    World_Position result = pos;

    recanonicalizeCoordinate(world, world->tile_map_count_x, world->count_x, &result.tile_map_x, &result.tile_x, &result.tile_rel_x);
    recanonicalizeCoordinate(world, world->tile_map_count_y, world->count_y, &result.tile_map_y, &result.tile_y, &result.tile_rel_y);

    return result;
}

/*
inline World_Position getCanonicalPosition(World* world, Canonical_Position* pos) {
    World_Position result;

    result.tile_map_x = pos->tile_map_x;
    result.tile_map_y = pos->tile_map_y;

    real32 x = pos->x - world->lower_left_x;
    real32 y = pos->y - world->lower_left_y;
    result.tile_x = floorReal32ToInt32(x / world->tile_side_in_pixels);
    result.tile_y = floorReal32ToInt32(y / world->tile_side_in_pixels);

    result.tile_rel_x = x - result.tile_x * world->tile_side_in_pixels;
    result.tile_rel_y = y - result.tile_y * world->tile_side_in_pixels;

    Assert(result.tile_rel_x >= 0);
    Assert(result.tile_rel_y >= 0);
    Assert(result.tile_rel_x < world->tile_side_in_pixels);
    Assert(result.tile_rel_y < world->tile_side_in_pixels);

    if (result.tile_x < 0) {
        result.tile_x = world->count_x + result.tile_x;
        result.tile_map_x--;
        if (result.tile_map_x < 0) {
            result.tile_map_x += world->tile_map_count_x;
        }
    }
    if (result.tile_y < 0) {
        result.tile_y = world->count_y + result.tile_y;
        result.tile_map_y--;
        if (result.tile_map_y < 0) {
            result.tile_map_y += world->tile_map_count_y;
        }
    }
    if (result.tile_x >= world->count_x) {
        result.tile_x = world->count_x - result.tile_x;
        result.tile_map_x++;
        if (result.tile_map_x >= world->tile_map_count_x) {
            result.tile_map_x -= world->tile_map_count_x;
        }
    }
    if (result.tile_y >= world->count_y) {
        result.tile_y = world->count_y - result.tile_y;
        result.tile_map_y++;
        if (result.tile_map_y >= world->tile_map_count_y) {
            result.tile_map_y -= world->tile_map_count_y;
        }
    }
    return result;
}
*/

static bool32 isWorldPointEmpty(World* world, World_Position* test_pos) {
    bool32 empty = false;
    Tile_Map* tile_map = getTileMap(world, test_pos->tile_map_x, test_pos->tile_map_y);
    empty = isTileMapPointEmpty(world, tile_map, test_pos->tile_x, test_pos->tile_y);
    return empty;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Game_State* game_state = (Game_State*)memory->permanent_storage;

    u32 tiles_00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = 
    {
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 1, 1, 1,   1, 1, 1, 1,   1,  1, 1, 1, 1,  0, 0, 0, 0},
        {0, 1, 1, 0,   0, 0, 1, 0,   0,  0, 1, 0, 1,  1, 1, 0, 0},
        {0, 0, 1, 0,   0, 0, 1, 0,   0,  0, 1, 0, 0,  1, 1, 0, 0},
        {1, 1, 1, 0,   0, 0, 1, 0,   0,  0, 1, 0, 0,  1, 1, 1, 1},
        {0, 0, 1, 1,   1, 1, 1, 1,   1,  0, 1, 0, 0,  1, 1, 0, 0},
        {0, 0, 1, 1,   1, 0, 0, 0,   1,  1, 1, 1, 1,  1, 1, 0, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  1, 1, 1, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
    };
    u32 tiles_01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = 
    {
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,   1, 1, 1, 1,   1,  1, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  1, 1, 0, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,   1, 1, 1, 1,   1,  1, 1, 1, 1,  1, 1, 1, 1},
        {0, 0, 0, 0,   0, 0, 0, 1,   1,  0, 1, 0, 0,  0, 0, 0, 0},
        {0, 0, 1, 1,   1, 0, 0, 0,   1,  1, 0, 0, 0,  1, 1, 0, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  1, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
    };

    u32 tiles_10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = 
    {
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 1, 1, 1,   1, 1, 1, 1,   1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 1, 0},
        {0, 1, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 1, 0},
        {1, 1, 1, 1,   1, 1, 1, 1,   1,  1, 1, 1, 1,  1, 1, 1, 1},
        {0, 1, 0, 0,   0, 0, 0, 1,   1,  0, 0, 0, 0,  0, 0, 1, 0},
        {0, 1, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 1, 1, 1,   1, 1, 1, 1,   1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
    };
    u32 tiles_11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = 
    {
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 1, 1, 1,   1, 1, 1, 1,   1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 0, 0,   0, 0, 0, 0,   0,  0, 0, 0, 0,  0, 0, 1, 0},
        {0, 1, 0, 0,   0, 0, 0, 0,   0,  0, 0, 0, 0,  0, 0, 1, 0},
        {1, 1, 0, 0,   0, 0, 0, 0,   0,  0, 0, 0, 0,  0, 0, 1, 1},
        {0, 1, 0, 0,   0, 0, 0, 1,   0,  0, 0, 0, 0,  0, 0, 1, 0},
        {0, 1, 0, 0,   0, 0, 0, 0,   0,  0, 0, 0, 0,  0, 0, 1, 0},
        {0, 1, 1, 1,   1, 1, 1, 1,   1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   1,  0, 0, 0, 0,  0, 0, 0, 0},
    };
    Tile_Map tile_maps[2][2];

    tile_maps[0][0].tiles = (u32*)tiles_00;
    tile_maps[0][1].tiles = (u32*)tiles_01;
    tile_maps[1][0].tiles = (u32*)tiles_10;
    tile_maps[1][1].tiles = (u32*)tiles_11;

    World world;
    world.tile_map_count_x = 2;
    world.tile_map_count_y = 2;
    world.count_x = TILE_MAP_COUNT_X;
    world.count_y = TILE_MAP_COUNT_Y;
    world.tile_map = (Tile_Map*)tile_maps;

    world.tile_side_in_meters = 1.0f;
    world.tile_side_in_pixels = 70;

    world.lower_left_x = 20;
    world.lower_left_y = 20;


    if(!memory->is_initialized)
    {
        char* filename = __FILE__;
        
        /*
        Debug_Read_File_Result file = memory->DEBUG_platformReadEntireFile(thread, filename);
        if(file.contents)
        {
            memory->DEBUG_platformWriteEntireFile(thread, "test.out", file.contents_size, file.contents);
            memory->DEBUG_platformFreeFileMemory(thread, file.contents);
        }
        */
        game_state->player_pos.tile_map_x = 0;
        game_state->player_pos.tile_map_y = 0;
        game_state->player_pos.tile_rel_x = 0.5f;
        game_state->player_pos.tile_rel_y = 0.5f;
        game_state->player_pos.tile_x = 1;
        game_state->player_pos.tile_y = 1;

        game_state->player_side_length = world.tile_side_in_meters / 6.0f;
        game_state->player_side_length_pixels = world.tile_side_in_pixels * (game_state->player_side_length / world.tile_side_in_meters);
        game_state->tone_hz = 512;
        game_state->player_speed_scaler = 10.0f;
        game_state->player_max_distance_per_frame = 1.0f;

        // TODO(casey): This may be more appropriate to do in the platform layer
        memory->is_initialized = true;
    }

    Tile_Map* active_tile_map = getTileMap(&world, game_state->player_pos.tile_map_x, game_state->player_pos.tile_map_y);
    
    //drawRectangle(buffer, 0.0f, 0.0f, (real32)buffer->width, (real32)buffer->height, 1.0f, 0.0f, 1.0f );
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

        World_Position new_player_pos =  game_state->player_pos;
        real32 player_x_diff = game_state->player_speed_scaler * input->dt_for_frame * controller->left_stick_average_x;
        real32 player_y_diff = game_state->player_speed_scaler * input->dt_for_frame * controller->left_stick_average_y;

        new_player_pos.tile_rel_x += clampAbsValueReal32(player_x_diff, game_state->player_max_distance_per_frame);
        new_player_pos.tile_rel_y += clampAbsValueReal32(player_y_diff, game_state->player_max_distance_per_frame);

        new_player_pos = recanonicalizePosition(&world, new_player_pos);

        //bool32 test = isWorldPointEmpty(&world, new_player_pos);
        bool32 update_pos = isWorldPointEmpty(&world, &new_player_pos);
        if(!update_pos){
            if (isWorldPointEmpty(&world, &new_player_pos)) {
                game_state->player_pos = new_player_pos;
            }
        }
        if (isWorldPointEmpty(&world, &new_player_pos)) {
            game_state->player_pos = new_player_pos;
        }
    }

    drawRectangle(buffer, 0.0f, 0.0f, (real32)buffer->width, (real32)buffer->height,
                  1.0f, 0.0f, 0.0f);

    int test_gradient = 0;
    for (int row = 0; row <= world.count_y; ++row) {
        for (int col = 0; col <= world.count_x; ++col) {
            u32 tile_id = getTileValueUnchecked(&world, active_tile_map, col, row);
            real32 gray = 0.4f;
            if(tile_id == 1) {
                gray = 1.0f;
            }
            gray *= ((real32)(test_gradient) / (real32)(world.count_x * world.count_y));
            real32 min_x = world.lower_left_x + ((real32)(col) * world.tile_side_in_pixels);
            real32 max_x = min_x + world.tile_side_in_pixels;

            real32 max_y = world.lower_left_y + ((real32)(world.count_y - row) * world.tile_side_in_pixels);
            real32 min_y = max_y - world.tile_side_in_pixels;
            drawRectangle(buffer, min_x, min_y, max_x, max_y, gray, gray, gray);

            test_gradient++;
        }
    }

    // Turn player pos into a pixel locaiton on the screen.
    int player_x_pixel = world.lower_left_x + 
                        (world.tile_side_in_pixels * game_state->player_pos.tile_x) + 
                        floorReal32ToInt32(game_state->player_pos.tile_rel_x * 
                                          ((real32)world.tile_side_in_pixels / world.tile_side_in_meters)); 
    int player_y_pixel = world.lower_left_y + 
                        (world.tile_side_in_pixels * game_state->player_pos.tile_y) + 
                        floorReal32ToInt32(game_state->player_pos.tile_rel_y * 
                                          ((real32)world.tile_side_in_pixels / world.tile_side_in_meters)); 
    renderPlayer(buffer, player_x_pixel, player_y_pixel, game_state->player_side_length_pixels);

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
#include "core.h"
// Bug with offset of pixel colors, everythign got darker.
//

u32 const TILE_MAP_COUNT_X = 16;
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

static void renderPlayer(Game_Offscreen_Buffer* buffer, int player_x, int player_y) {
    u8* end_of_buffer = (u8*)buffer->memory + (buffer->pitch * buffer->height);
    u32 color = 0xFF00ff00; //FFFFFF;
    
    int top = player_y;
    int bottom = player_y + 10;
    for (int x = player_x; x < player_x + 10; ++x) {
        u8* pixel = ((u8*)buffer->memory + (x * buffer->bytes_per_pixel) + (top * buffer->pitch));
        for (int y = top; y < bottom; ++y) {
            if ((pixel >= buffer->memory) && ((pixel + 4) <= end_of_buffer)) {
                *(u32*)pixel = color;
            }
            pixel += buffer->pitch;
        }
    }
    drawRectangle(buffer, (real32)player_x, (real32)player_y, (real32)(player_x + 10),(real32)(player_y + 10), 0.0f, 1.0f, 1.0f);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Game_State* game_state = (Game_State*)memory->permanent_storage;
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
       
        game_state->tone_hz = 512;
        game_state->t_sine = 0.0f;

        game_state->player_x = 50;
        game_state->player_y = 50;

        // TODO(casey): This may be more appropriate to do in the platform layer
        memory->is_initialized = true;
    }

    u32 tile_map[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = 
    {
        {1, 1, 1, 0,   0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0},
        {1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,  1, 0, 0, 0},
        {1, 1, 1, 0,   0, 0, 1, 0,   0, 0, 1, 0,  1, 1, 1, 0},
        {0, 0, 1, 0,   0, 0, 1, 0,   0, 0, 1, 0,  0, 1, 1, 0},
        {0, 1, 1, 0,   0, 0, 1, 0,   0, 0, 1, 0,  0, 1, 0, 0},
        {0, 0, 1, 1,   1, 1, 1, 1,   1, 0, 1, 0,  0, 1, 1, 0},
        {0, 0, 1, 1,   1, 0, 0, 0,   1, 1, 1, 1,  1, 1, 1, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   0, 1, 1, 1,  0, 0,- 0, 0},
        {0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,  0, 0, 0, 0},
    };

    real32 upper_left_x = 40;
    real32 upper_left_y = 40;
    real32 tile_width = 75;
    real32 tile_height = 75;
    
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
        float move_scale = 200.0f;

        int new_player_x = game_state->player_x + roundReal32ToInt32(move_scale * input->dt_for_frame * (controller->left_stick_average_x));
        int new_player_y = game_state->player_y + roundReal32ToInt32(move_scale * input->dt_for_frame * (controller->left_stick_average_y));
        int player_tile_x = truncateReal32ToInt32((new_player_x - upper_left_x) / tile_width);
        int player_tile_y = truncateReal32ToInt32((new_player_y - upper_left_y) / tile_height);

        bool32 is_valid = false;
        if ((player_tile_x >= 0) && (player_tile_x < TILE_MAP_COUNT_X) &&
            (player_tile_y >= 0) && (player_tile_y < TILE_MAP_COUNT_Y)) {
                u32 tile_map_value = tile_map[player_tile_y][player_tile_x];
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
        }


        //game_state->player_x += truncateReal32ToInt32(move_scale * (input->dt_for_frame) * (controller->left_stick_average_x));
        //game_state->player_y += truncateReal32ToInt32(move_scale * (input->dt_for_frame) * (controller->left_stick_average_y));


        if(game_state->t_jump > 0)
        {
            game_state->player_y += (int)(5.0f*sinf(0.5f*Pi32*game_state->t_jump));
        }
        if(controller->action_down.ended_down)
        {
            game_state->t_jump = 4.0;
        }
        game_state->t_jump -= 0.033f;
    }
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 16; ++col) {
            u32 tile_id = tile_map[row][col];
            real32 gray = 0.4f;
            if(tile_id == 1) {
                gray = 1.0f;
            }
            real32 min_x = upper_left_x + ((real32)col) * tile_width;
            real32 min_y = upper_left_y + ((real32)row) * tile_height;
            real32 max_x = min_x + tile_width;
            real32 max_y = min_y + tile_height;
            drawRectangle(buffer, min_x, min_y, max_x, max_y, gray, gray, gray);
        }
    }
    renderPlayer(buffer, game_state->player_x, game_state->player_y);

    //RenderPlayer(buffer, input->mouse_x, input->mouse_y);

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
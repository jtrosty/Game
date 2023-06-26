#include "core.h"

internal void
gameOutputSound(Game_State* game_state, Game_Sound_Output_Buffer* sound_buffer, int tone_hz)
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

        game_state->player_x = 200;
        game_state->player_y = 350;

        // TODO(casey): This may be more appropriate to do in the platform layer
        memory->is_initialized = true;
    }

    for(int controller_index = 0;
        controller_index < ArrayCount(input->controllers);
        ++controller_index)
    {
        Game_Controller_Input* controller = getController(input, controller_index);
        if(controller->is_analog)
        {
            // NOTE(casey): Use analog movement tuning
            game_state->blue_offset += (int)(4.0f*controller->left_stick_average_x);
            game_state->tone_hz = 512 + (int)(128.0f*controller->left_stick_average_y);
        }
        else
        {
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

        game_state->player_x += (int)(4.0f*controller->left_stick_average_x);
        game_state->player_y += (int)(4.0f*controller->left_stick_average_y);
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
    
    renderWeirdGradient(buffer, game_state->blue_offset, game_state->green_offset);
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
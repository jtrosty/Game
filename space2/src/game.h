#pragma once

void game_create_window( 
    const unsigned short in_width,
    const unsigned short in_height,
    const char* in_title
);

void game_execute_loop();
void game_window_should_close();
void game_close_window();



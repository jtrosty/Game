#include "core.h"
#include "raylib.h" 
#define RAYLIB_IMPLEMENTATION 
#include "raygui.h"
// Only for Hot Reload
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
    /* Below are the functions pointers we need to make the bridge
    between the main.c (which contains raylib and raygui)
    and this core.c which will be converted to a dll.*/
    /* To be short, it's to be able to call raylib and raygui
    functions even if we are not linking them as static libraries
    into this dll. */
    // Raylib functions
    void (*raylib_init_window)(int width, int height, const char* title);
    void (*raylib_close_window)();
    bool (*raylib_window_should_close)();
    void (*raylib_begin_drawing)();
    void (*raylib_end_drawing)();
    void (*raylib_clear_background)(Color color);
    // Raygui functions
    bool (*raygui_gui_button)(Rectangle bounds, const char *text);
    void (*raygui_gui_set_style)(int control, int property, int value);
    // This function will be one of the first one to be load 
    // from this dll in main.c. It's role is actually to initialize
    // our raylib and raygui functions pointers, to create the bridge.
    CORE void core_load_raylib_functions(
        void (*const in_init_window)(int width, int height, const char* title),
        void (*const in_close_window)(),
        bool (*const in_window_should_close)(),
        void (*const in_begin_drawing)(),
        void (*const in_end_drawing)(),
        void (*const in_clear_background)(Color color),
        bool (*const in_gui_button)(Rectangle bounds, const char *text),
        void (*const in_gui_set_style)(int control, int property, int value)
    ){
        // Raylib functions
        raylib_init_window = in_init_window;
        raylib_close_window = in_close_window;
        raylib_window_should_close = in_window_should_close;
        raylib_begin_drawing = in_begin_drawing;
        raylib_end_drawing = in_end_drawing;
        raylib_clear_background = in_clear_background;
        // Raygui functions
        raygui_gui_button = in_gui_button;
        raygui_gui_set_style = in_gui_set_style;
    }
    // This variable determines if we need to activate hot reload
    // function or not from the main loop, inside main.c
    // That could be a bool, of course, but I'm not including
    // stdbool.h. So, 0 if false, 1 if true.
    char core_active_hot_reload = 0;
    CORE void core_get_value_hot_reload(char* out_index){
        *out_index = core_active_hot_reload;
    }
#else
    // I redefine the raylib and raygui functions
    // I use here, because whenever we are using the dll
    // or not, I want to be able to have one single code,
    // without #if/#else in the functions below.
    
    // Raylib functions
    #define raylib_init_window InitWindow
    #define raylib_close_window CloseWindow
    #define raylib_window_should_close WindowShouldClose
    #define raylib_begin_drawing BeginDrawing
    #define raylib_end_drawing EndDrawing
    #define raylib_clear_background ClearBackground
    // Raygui functions
    #define raygui_gui_button GuiButton
    #define raygui_gui_set_style GuiSetStyle
#endif
void core_create_window(
    const unsigned short in_width,
    const unsigned short in_height,
    const char* in_title
){
    raylib_init_window(in_width, in_height, in_title);
}
extern "C" void core_execute_loop(){
    // Only for Hot Reload
    #if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
        unsigned int key_pressed = raylib_get_key_pressed();
        // In my case, if the key pressed is the 'R', I perform
        // hot reload
        if(key_pressed == 114){
            core_active_hot_reload = 1;
        }
    #endif
    raylib_begin_drawing();
        raylib_clear_background(RED);
        if(raygui_gui_button((Rectangle){ 500, 200, 250, 60 }, "TEST BUTTON")) {
            puts("Button pressed\n");
        }
    raylib_end_drawing();
}
extern "C" char core_window_should_close(){
    return raylib_window_should_close();
}
void core_close_window(){
    raylib_close_window();
}
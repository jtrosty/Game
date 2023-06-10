#include "core.h"
// Whenever we will pass this option to the compiler, we will be able to use our core.c code as a dll
#if defined(CORE_USE_LIBTYPE_SHARED) 
    #include <libloaderapi.h>
    #include "raylib.h"
    #include "raygui.h"
#endif
int main(){
    // This code is necessary only if we are using hot reload
    #if defined(CORE_USE_LIBTYPE_SHARED)
        HINSTANCE dll_handle = NULL; // Declare a pointer to the dll resource
        // We need to initialize some function pointers, because, 
        // as mentioned before, by using a dll, we will load every functions
        // we need to use from raylib, to our dll, (think of it as a bridge), 
        // because our dll is not aware of raylib. 
        // We are not linking its static library in our dll file.
        void (*core_load_raylib_functions_func)(
            void (*const in_init_window)(int width, int height, const char* title),
            void (*const in_close_window)(),
            void (*const in_begin_drawing)(),
            void (*const in_end_drawing)(),
            void (*const in_clear_background)(Color color),
            int (*const in_get_key_pressed)(),
            bool (*const in_is_key_down)(int key),
            bool (*const in_gui_button)(Rectangle bounds, const char *text),
        );
        // Functions pointers to be able to store and call functions from 
        // our dll file (the initialization of the window, the exec loop function...)
        void (*core_init_window_func)();
        void (*core_execute_loop_func)();
        void (*core_get_hot_reload_func)(char* out_index);
        void (*core_exit_func)();
 
        // To be able to know if we need to perform hot reload
        // (0 = false, 1 = true)
        char activate_hot_reload = 0;
        // Load our core.dll file as a dynamic library.
        // Regarding to the official windows documentation, the second parameter of 'LoadLibraryExA'
        // is always NULL (just here for future need), and the last one corresponds to a flag
        // which determine the action to be taken when loading the module. In our case, we use the 
        // default action (so the flag is 0)
        dll_handle = LoadLibraryExA("./core.dll", NULL, 0);
        if(dll_handle != NULL){
            // Load the 'core_load_raylib_functions' function from the dll into the
            // 'core_load_raylib_functions_func' function pointer
            core_load_raylib_functions_func = (void*) GetProcAddress(dll_handle, "core_load_raylib_functions");
            if (NULL == core_load_raylib_functions_func){
                printf("Can't call core_load_raylib_functions dll function");
                exit(1);
            }else{
                // If the function is correclty loaded, then we call it, 
                // and we pass the functions from raylib and raygui
                // we need.
                // It's the bridge between our main.c and our dll, to 
                // retrieve the raylib and raygui functions.
                core_load_raylib_functions_func(
                    &InitWindow,
                    &CloseWindow,
                    &BeginDrawing,
                    &EndDrawing,
                    &ClearBackground,
                    &GuiButton,
                    &GuiSetStyle
                );
            }
            core_init_window_func = (void*) GetProcAddress(dll_handle, "core_init_window");
            if (NULL == core_init_window_func){
                printf("Can't call core_init_window dll function");
                exit(1);
            }else{
                // If the 'core_init_window' function is correclty
                // loaded from the dll, we call it right now to
                // initialize the window.
                core_init_window_func();
            }
            core_execute_loop_func = (void*) GetProcAddress(dll_handle, "core_execute_loop");
            if (NULL == core_execute_loop_func){
                printf("Can't call core_execute_loop dll function");
                exit(1);
            }
            core_get_hot_reload_func = (void*) GetProcAddress(dll_handle, "core_get_hot_reload");
            if(NULL == core_get_hot_reload_func){
                printf("Can't call core_get_hot_reload dll function");
                exit(1);
            }
            core_exit_func = (void*) GetProcAddress(dll_handle, "core_exit");
            if (NULL == core_exit_func){
                printf("Can't call core_execute_loop dll function");
                exit(1);
            }
        }else{
            // A problem occured when trying to load the dll file.
            // The most common mistake is a wrong given path to
            // the dll file.
            printf("Can't load the dll file.\n");
            exit(1);
        }
    #else
        // If we are not using the dll configuration, 
        // then we just need to call the 'core_init_window' function
        // from core.c
        core_init_window();
    #endif
    // Main loop
    while(1){
        #if defined(CORE_USE_LIBTYPE_SHARED)
            // We call the function loaded from the dll
            core_execute_loop_func();
            core_get_hot_reload_func(&activate_hot_reload);
            if(activate_hot_reload == 1){
                // First, we free the library
                FreeLibrary(dll_handle);
                // Then, we demand to the user to press a touch to continue 
                // (enter key, on the terminal which runs the main program)
                char c;
                puts("[INFO] Press enter after rebuilding the functions dll file\n");
                // While the user doesn't pres any key, the scanf "block" the program
                scanf("%c", &c);
                
                // If the user press enter, we need to reload the dll file, and the functions we need
                dll_handle = LoadLibraryExA("core.dll", NULL, 0);
                if (NULL != dll_handle){
                    core_load_raylib_functions_func = (void*) GetProcAddress(dll_handle, "core_load_raylib_functions");
                    if (NULL == core_load_raylib_functions_func){
                        printf("Can't call core_load_raylib_functions dll function");
                        exit(1);
                    }else{
                        core_load_raylib_functions_func(
                            &InitWindow,
                            &CloseWindow,
                            &BeginDrawing,
                            &EndDrawing,
                            &ClearBackground,
                            &GuiButton,
                            &GuiSetStyle
                        );
                    }
                    core_execute_loop_func = (void*) GetProcAddress(dll_handle, "core_execute_loop");
                    if (NULL == core_execute_loop_func){
                        printf("Can't call core_execute_loop dll function");
                        exit(1);
                    }
                    core_get_hot_reload_func = (void*) GetProcAddress(dll_handle, "core_get_hot_reload");
                    if(NULL == core_get_hot_reload_func){
                        printf("Can't call core_get_hot_reload dll function");
                        exit(1);
                    }
                    core_exit_func = (void*) GetProcAddress(dll_handle, "core_exit");
                    if (NULL == core_exit_func){
                        printf("Can't call core_execute_loop dll function");
                        exit(1);
                    }
                }else{
                    printf("Can't load the dll file.\n");
                    exit(1);
                }
            }
        #else
            // If we are not using the dll configuration, 
            // we direclty call the function from core.c 
            core_exec_loop();
        #endif
    }
    #if defined(CORE_USE_LIBTYPE_SHARED)
        core_exit_func();
        // Important line here, we need to free the dll
        FreeLibrary(dll_handle);
    #else
        // If we are not using the dll configuration,
        // simply call the 'core_exit' function from core.c
        core_exit();
    #endif
    return 0;
}
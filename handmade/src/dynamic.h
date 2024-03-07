#if !defined(CORE_DYNAMIC_H)

#include <dlfcn.h>

#if !WIN_RAY

static Ray_Game_Code* mac_LoadDllGameCode(const char* game_dll_full_path, const char* temp_dll_full_path) {

    Ray_Game_Code* result = {};

    result->dll_last_write_time = GetFileModTime(game_dll_full_path);
    if (result->dll_last_write_time) {
        printf("We got the result");
    }

    // Time to copy file
    unsigned int loaded_dll_bytes_read = 0;
    
    // Load file data as byte array (read)
    unsigned char* loaded_dll = LoadFileData(game_dll_full_path, &loaded_dll_bytes_read);       
    if (!SaveFileData(temp_dll_full_path, loaded_dll, loaded_dll_bytes_read)) {
        printf("Failed to save temp dll\n");
    }   // Save data to file from byte array (write), returns true on success

    // Use the copied dll for running so that way the original is available for writing when rebuilding
    result->game_code_dll = dlopen(temp_dll_full_path, RTLD_NOW);

    if (result->game_code_dll) {
        result->update_and_render_fn = (game_update_and_render*)dlsym(result->game_code_dll, "GameUpdateAndRender");
        result->get_sound_samples_fn = (game_get_sound_samples*)dlsym(result->game_code_dll, "GameGetSounddSamples");
        result->is_valid = (result->update_and_render_fn && result->get_sound_samples_fn);
    }

    // If there is a failure, set the functions pointers to zero.
    if (!result->is_valid) {
        result->update_and_render_fn = 0;
        result->get_sound_samples_fn = 0;
    }

    return result;
}
// Not Tested
static void mac_unloadGameCode(Ray_Game_Code* game_code) {
    if (game_code->game_code_dll) {
        dlclose(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }
    game_code->is_valid = false;
    game_code->update_and_render_fn = 0;
    game_code->get_sound_samples_fn = 0;
}

inline FILETIME win32_getLastWriteTime(char* filename) {
    FILETIME last_write_time = {};
    WIN32_FILE_ATTRIBUTE_DATA data;
    // TODO CHANGED for CLANG 
#if CLANG_COMPILE
    if (GetFileAttributesEx((LPCWSTR)filename, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    } 
    else {

    }
#else 
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    } 
    else {

    }

#endif
    return last_write_time;
}
#else
#include <windows.h>
#include "win32_platform.h"

static Win32_Game_Code win32_loadGameCode(char* source_dll_name, char* temp_dll_name) {
    Win32_Game_Code result = {};
    //result.dll_last_write_time = win32
    result.dll_last_write_time = win32_getLastWriteTime(source_dll_name);

    //TODO CHANGED ADDED LPCWSTR
#if CLANG_COMPILE
    CopyFile((LPCWSTR)source_dll_name, (LPCWSTR)temp_dll_name, FALSE);
#else 
    CopyFile(source_dll_name, temp_dll_name, FALSE);
    
#endif

    result.game_code_dll = LoadLibraryA(temp_dll_name);

    if (result.game_code_dll) {
        result.update_and_render_fn = (game_update_and_render*)
            GetProcAddress(result.game_code_dll, "GameUpdateAndRender");
        result.get_sound_samples_fn = (game_get_sound_samples*)
            GetProcAddress(result.game_code_dll, "GameGetSoundSamples");

        result.is_valid = (result.update_and_render_fn && result.update_and_render_fn);
    }

    if (!result.is_valid) {
        result.update_and_render_fn = 0;
        result.get_sound_samples_fn = 0;
    }
    
    return result;
}

static void Win32_unloadGameCode(Win32_Game_Code* game_code) {
    if (game_code->game_code_dll) {
        FreeLibrary(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }
    game_code->is_valid = false;
    game_code->update_and_render_fn = 0;
    game_code->get_sound_samples_fn = 0;
}
#endif

#define CORE_DYNAMIC_H
#endif

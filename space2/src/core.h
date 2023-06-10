#pragma once
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
    #define CORE __declspec(dllexport) // We are building core as a Win32 shared library (.dll)
#elif defined(_WIN32) && defined(USE_CORE_LIB_SHARED)
    #define CORE __declspec(dllimport) // We are using core as a Win32 shared library (.dll)
#else
    #define CORE // We are building or using core as a static library
#endif
void core_create_window(
    const unsigned short in_width,
    const unsigned short in_height,
    const char* in_title
);
extern "C" void core_execute_loop();
char core_window_should_close();
void core_close_window();
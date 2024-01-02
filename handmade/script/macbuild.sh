set -xe

WARNING_FLAGS="-Wall -Wextra -Wpedantic"
LIBS="'pkg-config --libs raylib' -lglfw lm ldl lpthred"
LINK_FLAGS="-framework OpenGL -framework OpenAL -framework IOKit -framework CoreVideo -framework Cocoa"
MAC_LINK_FLAGS="-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
COMPILATION_FLAGS="-std=c++17 -O0 -g -undefined dynamic_lookup"
OUTPUT_DIR="build"

ROOT_DIR=$PWD
RAYLIB_SRC="$ROOT_DIR/$RAYLIB_SRC"

CC=clang++

GAME_NAME="ray_core"

HANDMADE_WARNING_FLAGS="-Wno-deprecated-declarations -Wno-unused-function -Wno-unused-variable -Wno-c++11-narrowing -Wno-missing-braces -Wno-logical-not-parentheses -Wno-switch -Wno-write-strings -Wno-c++11-compat-deprecated-writable-strings -Wno-tautological-compare -Wno-missing-braces -Wno-null-dereference -Wno-writable-strings"

# Set your sources here (relative paths!)
# Example with two source folders:
# SOURCES="src/*.c src/submodule/*.c"
SOURCES_PLATFORM="../src/ray/ray_platform.cpp"
SOURCE="../src/"
EXTERNAL="../external"
EXTERNAL_FULL="/Users/jonathantrost/Documents/code/warfare/Game/handmade/external/libraylib.dylib"
EXTERNAL_DIR="/Users/jonathantrost/Documents/code/warfare/Game/handmade/external/libraylib.a"
OTHER_LIB="/usr/local/lib/"

EXTERNAL_IN="/Users/jonathantrost/Documents/code/warfare/Game/handmade/external"

# Set your raylib/src location here (relative path!)
RAYLIB_LIB="/Users/jonathantrost/Documents/code/raylib/src/"
FULL_RAYLIB_SRC="/Users/jonathantrost/Documents/code/raylib/src"
TRY_RAY=" ~/Documents/code/raylib/src"

RAYLIB_DEFINES="-D_DEFAULT_SOURCE -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33"
RAYLIB_C_FILES="$RAYLIB_SRC/rcore.c $RAYLIB_SRC/rshapes.c $RAYLIB_SRC/rtextures.c $RAYLIB_SRC/rtext.c $RAYLIB_SRC/rmodels.c $RAYLIB_SRC/utils.c $RAYLIB_SRC/raudio.c"
RAYLIB_INCLUDE_FLAGS="-I$FULL_RAYLIB_SRC -I$RAYLIB_SRC/external/glfw/include -I$EXTERNAL_IN"
#DYLD_LIBRARY_PATH="$RAYLIB_SRC/libraylib.dylib"
#DYLD_LIBRARY_PATH="/usr/local/lib/libraylib.450.dylib"
DYLD_LIBRARY_PATH="$EXTERNAL_IN/libraylib.dylib"
STATIC_RAY_LIB_PATH="$RAYLIB_SRC/libraylib.a"

#$CC -c $RAYLIB_DEFINES $RAYLIB_INCLUDE_FLAGS $COMPILATION_FLAGS -x objective-c $RAYLIB_SRC/rglfw.c > /dev/null 2>&1
#$CC -c $RAYLIB_DEFINES $RAYLIB_INCLUDE_FLAGS $COMPILATION_FLAGS $RAYLIB_C_FILES > /dev/null 2>&1

#$(CXX) $(COPTS) $(HANDMADE_COMPILER_FLAGS) $(HANDMADE_WARNING_FLAGS) $(CPP11_FLAGS) -dynamiclib -o $@ $(HANDMADE_CODE_PATH)/handmade.cpp

#        TSODING stuff
pwd
pushd build
if test -f ray_platform; then
    rm ray_platform
fi
if test -f temp_ray_core.dylib; then
    rm temp_ray_core.dylib
fi
if test -f ray_core.dylib; then
    rm ray_core.dylib
fi

pwd
#clang++ -v -dynamiclib -o ray_core.dylib "$SOURCE/core.cpp" -L $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS
clang++ -v -dynamiclib -o ray_core.dylib "$SOURCE/core.cpp" $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS $COMPILATION_FLAGS
clang++ -v -o ray_platform $SOURCES_PLATFORM  $FULL_RAYLIB_SRC/libraylib.a -I $RAYLIB_LIB $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS #$RAYLIB_DEFINES 
#clang++ -v -o ray_platform_static $SOURCES_PLATFORM  $EXTERNAL_DIR  $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS #$RAYLIB_DEFINES 
popd
#clang++ $CFLAGS -o ./ray_platform ./src/ray/rayplatform.cpp $LIBS -L.?build/
#clang -o ./build/mac ./src/.c -lm
#
#clang++ -v -dynamiclib -o ray_core.dylib "$SOURCE/core.cpp" -L $FULL_RAYLIB_SRC $DYLD_LIBRARY_PATH $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS
#clang++ -v -o ray_platform $SOURCES_PLATFORM  $FULL_RAYLIB_SRC/libraylib.a -I $RAYLIB_LIB $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS #$RAYLIB_DEFINES 
#
#
#dynamic
#clang++ -v -dynamiclib -o ray_core.dylib "$SOURCE/core.cpp" -I $OTHER_LIB -L $FULL_RAYLIB_SRC $DYLD_LIBRARY_PATH $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS
#clang++ -v -o ray_platform $SOURCES_PLATFORM -L $FULL_RAYLIB_SRC -I export $DYLD_LIBRARY_PATH  $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS $HANDMADE_WARNING_FLAGS #$RAYLIB_DEFINES 

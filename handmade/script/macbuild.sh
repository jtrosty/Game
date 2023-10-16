set -xe

WARNING_FLAGS="-Wall -Wextra -Wpedantic"
LIBS="'pkg-config --libs raylib' -lglfw lm ldl lpthred"
LINK_FLAGS="-framework OpenGL -framework OpenAL -framework IOKit -framework CoreVideo -framework Cocoa"
MAC_LINK_FLAGS="-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
COMPILATION_FLAGS="-std=c++17 -O0 -g" #-undefined dynamic_lookup "
OUTPUT_DIR="build"

ROOT_DIR=$PWD
SOURCES="$ROOT_DIR/$SOURCES"
RAYLIB_SRC="$ROOT_DIR/$RAYLIB_SRC"

CC=clang++

GAME_NAME="ray_core"

# Set your sources here (relative paths!)
# Example with two source folders:
# SOURCES="src/*.c src/submodule/*.c"
SOURCES="../src/ray/ray_platform.cpp"
EXTERNAL="../external"
EXTERNAL_FULL="/Users/jonathantrost/Documents/code/warfare/Game/handmade/external/libraylib.dylib"
EXTERNAL_FULL="/Users/jonathantrost/Documents/code/warfare/Game/handmade/external/libraylib.dylib"

EXTERNAL_IN="/Users/jonathantrost/Documents/code/warfare/Game/handmade/external/"

# Set your raylib/src location here (relative path!)
RAYLIB_SRC="/Users/jonathantrost/Documents/code/raylib/src"
FULL_RAYLIB_SRC="/Users/jonathantrost/Documents/code/raylib/src"
TRY_RAY=" ~/Documents/code/raylib/src"

RAYLIB_DEFINES="-D_DEFAULT_SOURCE -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33"
RAYLIB_C_FILES="$RAYLIB_SRC/rcore.c $RAYLIB_SRC/rshapes.c $RAYLIB_SRC/rtextures.c $RAYLIB_SRC/rtext.c $RAYLIB_SRC/rmodels.c $RAYLIB_SRC/utils.c $RAYLIB_SRC/raudio.c"
RAYLIB_INCLUDE_FLAGS="-I$FULL_RAYLIB_SRC -I$RAYLIB_SRC/external/glfw/include -I$EXTERNAL_IN"
DYLD_LIBRARY_PATH="$RAYLIB_SRC/libraylib.dylib"

#$CC -c $RAYLIB_DEFINES $RAYLIB_INCLUDE_FLAGS $COMPILATION_FLAGS -x objective-c $RAYLIB_SRC/rglfw.c > /dev/null 2>&1
#$CC -c $RAYLIB_DEFINES $RAYLIB_INCLUDE_FLAGS $COMPILATION_FLAGS $RAYLIB_C_FILES > /dev/null 2>&1


#        TSODING stuff
pwd
pushd build
pwd
clang++ -v -o core $SOURCES -L $FULL_RAYLIB_SRC -I export $DYLD_LIBRARY_PATH  $COMPILATION_FLAGS $MAC_LINK_FLAGS $RAYLIB_INCLUDE_FLAGS #$RAYLIB_DEFINES 
popd
#clang++ $CFLAGS -o ./ray_platform ./src/ray/rayplatform.cpp $LIBS -L.?build/
#clang -o ./build/mac ./src/.c -lm

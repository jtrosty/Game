MAC_LINK_FLAGS="-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
INCLUDES="-I /Users/jonathantrost/code/libs/glfw/include -I external"
GLFW="/Users/jonathantrost/code/libs/glfw/build/src/libglfw3.a"
SRC="main.cpp glad.c -o main"
$WARNINGS="-Wdeprecated"


clang++ $SRC $GLFW $INCLUDES $MAC_LINK_FLAGS

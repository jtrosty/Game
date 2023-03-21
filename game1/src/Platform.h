#include "../include/raylib.h"

class Platform {

    public:
    // ----------------------------------------------------------------
    // ----------------------------------------------------------------
    Platform(int screenWidth, int screenHeight, char* title, int targetFPS);
    ~Platform();
    
    // ----------------------------------------------------------------
    //void updateMouse(MouseInput& mouseInput);
};
/*
    struct {
        int isDown;
        int isUp;
        int changed;
    } typedef PlatformButton;

    enum {
        mouseButtonLeft,
        mouseButtonRight,
        mouseButtonCount,
    };

    struct {
        Vector2 mousePos;
        PlatformButton buttonsTrost[mouseButtonCount];
    } typedef MouseInput;

    MouseInput mouseInputController;

*/
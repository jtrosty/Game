#include "Platform.h"

Platform::Platform(int screenWidth, int screenHeight, char* title, int targetFPS) {
    InitWindow(screenWidth, screenHeight, title);
    SetTargetFPS(targetFPS);
}

Platform::~Platform() {
    CloseWindow();
}

/*
void Platform::updateMouse(MouseInput& mouseInput) {
    if (IsMouseButtonPressed(mouseButtonLeft)) {
        if (mouseInputController.buttonsTrost(mouseButtonLeft) == 0) mouseInputController.buttons(mouseButtonLeft).changed = 1;
        else mouseInputController.buttonsTrost(mouseButtonLeft).changed = 0;
        mouseInputController.buttonsTrost(mouseButtonLeft).is_down = 1;
    }
    else if (IsMouseButtonRelease(mouseButtongLeft)) {
        if (mouseInputController.buttonsTrost(mouseButtonLeft) == 1) mouseInputController.buttons(mouseButtonLeft).changed = 1;
        else mouseInputController.buttonsTrost(mouseButtonLeft).changed = 0;
        mouseInputController.buttonsTrost(mouseButtonLeft).is_down = 0;
    }
    if (IsMouseButtonPressed(mouseButtonRight)) {
        if (mouseInputController.buttonsTrost(mouseButtonRight) == 0) mouseInputController.buttonsTrost(mouseButtonRight).changed = 1;
        else mouseInputController.buttonsTrost(mouseButtonRight).changed = 0;
        mouseInputController.buttonsTrost(mouseButtonRight).is_down = 1;
    }
    else if (IsMouseButtonRelease(mouseButtonRight)) {
        if (mouseInputController.buttonsTrost(mouseButtonRight) == 1) mouseInputController.buttons(mouseButtonRight).changed = 1;
        else mouseInputController.buttonsTrost(mouseButtonRight).changed = 0;
        mouseInputController.buttonsTrost(mouseButtonRight).is_down = 0;
    }
    mouseInputController.mousePos = GetMousePosition();
}
    */

#include "Platform.h"
#include "../include/raylib.h"

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;

    Platform platform(screenWidth, screenHeight, "Trosty Games", 30);

    Vector2 ballPosition = { 100.0f, 100.0f };
    Color ballColor = DARKBLUE;

    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
        ballPosition = GetMousePosition();
        // Update Input
        // ----------------------------------------------
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ballColor = MAROON;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) ballColor = LIME;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) ballColor = DARKBLUE;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_SIDE)) ballColor = PURPLE;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_EXTRA)) ballColor = YELLOW;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_FORWARD)) ballColor = ORANGE;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_BACK)) ballColor = BEIGE;
        // Draw
        // ----------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawCircleV(ballPosition, 40, ballColor);

            DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        // ----------------------------------------------
    }

    CloseWindow();

    return 0;
}

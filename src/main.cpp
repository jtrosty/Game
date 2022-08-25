#include "Platform.h"
#include "../include/raylib.h"

#define MOUSE_SCALE_MARK_SIZE 12

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;

    Platform platform(screenWidth, screenHeight, "Trosty Games", 30);

    Vector2 ballPosition = { 100.0f, 100.0f };
    Color ballColor = DARKBLUE;

    Rectangle rec = {100, 100, 200, 80};
    Vector2 mousePosition = { 0 };

    bool mouseScaleReady = false;
    bool mouseScaleMode = false;
    bool makeCircle = false;

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
        
        // ----------------------------------------------
        mousePosition = GetMousePosition();

        if (CheckCollisionPointRec(mousePosition, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            makeCircle = true;
        }
        else makeCircle = false;

        if (CheckCollisionPointRec(mousePosition, (Rectangle) {rec.x + rec.width - MOUSE_SCALE_MARK_SIZE, 
                                                                rec.y + rec.height - MOUSE_SCALE_MARK_SIZE,
                                                               MOUSE_SCALE_MARK_SIZE, MOUSE_SCALE_MARK_SIZE })) {
            mouseScaleReady = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) mouseScaleMode = true;
        }
        else mouseScaleReady = false;

        if (mouseScaleMode) {
            mouseScaleReady = true;
            
            rec.width = (mousePosition.x - rec.x);
            rec.height = (mousePosition.y - rec.y);
            
            // Check Minimum Rec Size
            if (rec.width < MOUSE_SCALE_MARK_SIZE) rec.width = MOUSE_SCALE_MARK_SIZE;
            if (rec.height < MOUSE_SCALE_MARK_SIZE) rec.height = MOUSE_SCALE_MARK_SIZE;

            // Check Max rec size
            if (rec.width > (GetScreenWidth() - rec.x)) rec.width = GetScreenWidth() - rec.x;
            if (rec.height > (GetScreenHeight() - rec.y)) rec.height = GetScreenHeight() - rec.y;

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) mouseScaleMode = false;
        }
        // Draw
        // ----------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawCircleV(ballPosition, 40, ballColor);
            DrawRectangleRounded(rec, 0.5f, 30, Fade(GREEN, 0.5f));

            if (makeCircle) {
                DrawCircleGradient((int)ballPosition.x, (int)ballPosition.y, 60.0f, RED, ORANGE);
            }

            if (mouseScaleReady) {
                DrawRectangleLinesEx(rec, 1, RED);
                DrawTriangle((Vector2) { rec.x + rec.width - MOUSE_SCALE_MARK_SIZE, rec.y + rec.height},
                             (Vector2) { rec.x + rec.width, rec.y + rec.height }, 
                             (Vector2) { rec.x + rec.width, rec.y + rec. height - MOUSE_SCALE_MARK_SIZE},
                             RED);
            }

            DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        // ----------------------------------------------
    }

    CloseWindow();

    return 0;
}

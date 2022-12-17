#pragma once
#include "Platform.h"
#include "../include/raylib.h"
#include "render.h"

#define MOUSE_SCALE_MARK_SIZE 12
#define BOARD_WIDTH 5
#define BOARD_HEIGTH 5


int main(void) {
    const int screenWidth = 1800;
    const int screenHeight = 1440;

    Platform platform(screenWidth, screenHeight, "Trosty Games", 30);
    Render renderer;

    Vector2 ballPosition = { 100.0f, 100.0f };
    Color ballColor = DARKBLUE;

    Rectangle rec = {200, 200, 200, 80};
    Vector2 mousePosition = { 0 };
    
    
    // Game Data:
    // Game board test
    //GameData* gameBoard = new GameData[40];
    BoardGameData gameBoard[BOARD_HEIGTH * BOARD_WIDTH] {0};
    UnitData heroUnits[(BOARD_HEIGTH * BOARD_WIDTH) / 2] {0};
    UnitData villian[(BOARD_HEIGTH * BOARD_WIDTH) / 2] {0};


    // Initialize the Board Data
    const int startBkgd = 10;
    const int width = 210;
    const int height = 150;

    for (int i = 0; i < 40; i++) {
        gameBoard[i].x = startBkgd + width * (i % 5); 
        gameBoard[i].y = startBkgd + height * (i / 5); 
        gameBoard[i].width = width;
        gameBoard[i].height = height;
        switch (i / 5) 
        {
        case 0:
            gameBoard[i].color = BLUE;
            break;
        case 1:
            gameBoard[i].color = GREEN;
            break;
        case 2:
            gameBoard[i].color = SKYBLUE;
            break;
        case 3:
            gameBoard[i].color = LIGHTGRAY;
            break;
        case 4:
            gameBoard[i].color = BEIGE;
            break;
        case 5:
            gameBoard[i].color = BROWN;
            break;
        case 6:
            gameBoard[i].color = ORANGE;
            break;
        case 7:
            gameBoard[i].color = RED;
            break;
        case 8:
            gameBoard[i].color = BLUE;
            break;
        default:
            break;
        }
    }


    // #################################################### Load file
    Image hobgoblinImage = LoadImage("../data/art/hobgoblin_infantry.png");
    Texture2D hobgoblinTex = LoadTextureFromImage(hobgoblinImage);



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

        /*
        if (CheckCollisionPointRec(mousePosition, (Rectangle) {rec.x + rec.width - MOUSE_SCALE_MARK_SIZE, 
                                                                rec.y + rec.height - MOUSE_SCALE_MARK_SIZE,
                                                               MOUSE_SCALE_MARK_SIZE, MOUSE_SCALE_MARK_SIZE })) {
            mouseScaleReady = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) mouseScaleMode = true;
        }
        else mouseScaleReady = false;
        */

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

        // Draw // ----------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            //TODO: Take this out to a just a draw function thatwill control background and player.  
            renderer.test_drawGameBoard(gameBoard);


            // ##################################################### Draw Circle
            DrawCircleV(ballPosition, 40, ballColor);
            DrawRectangleRounded(rec, 0.5f, 30, Fade(GREEN, 0.5f));

            if (makeCircle) {
                DrawCircleGradient((int)ballPosition.x, (int)ballPosition.y, 60.0f, RED, ORANGE);
            }

            if (mouseScaleReady) {
                DrawRectangleLinesEx(rec, 1, RED);
                /*
                DrawTriangle((Vector2) { rec.x + rec.width - MOUSE_SCALE_MARK_SIZE, rec.y + rec.height},
                             (Vector2) { rec.x + rec.width, rec.y + rec.height }, 
                             (Vector2) { rec.x + rec.width, rec.y + rec. height - MOUSE_SCALE_MARK_SIZE},
                             RED);
                             */
            }

            DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);

            // ########################### Test Code
            DrawTexture(hobgoblinTex, 10, 10, WHITE);


        EndDrawing();
        // ----------------------------------------------
    }
    // ###################### De-Initialization
    UnloadTexture(hobgoblinTex);  

    CloseWindow();

    return 0;
}

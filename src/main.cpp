//#pragma once
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
    //GameData* board = new GameData[40];
    BoardData board[BOARD_HEIGTH * BOARD_WIDTH] {0};
    Tile tiles[BOARD_HEIGTH * BOARD_WIDTH] {0};
    board->tiles = tiles;
    board->numRows = 8;
    board->numCols = 5;
    board->offset = 10;
    board->pixelHeight = 80;
    board->pixelWidth = 150;

    UnitData heroUnits[(BOARD_HEIGTH * BOARD_WIDTH) / 2] {0};
    UnitData villian[(BOARD_HEIGTH * BOARD_WIDTH) / 2] {0};

    // Initialize the Board Data
    const int startBkgd = 10;

    for (int i = 0; i < (board->numCols * board->numRows); i++) {
        board->tiles[i].x = startBkgd + board->pixelWidth * (i % board->numCols); 
        board->tiles[i].y = startBkgd + board->pixelHeight * (i / board->numCols); 
        switch (i / board->numCols) 
        {
        case 0:
            board->tiles[i].color = BLUE;
            break;
        case 1:
            board->tiles[i].color = GREEN;
            break;
        case 2:
            board->tiles[i].color = SKYBLUE;
            break;
        case 3:
            board->tiles[i].color = LIGHTGRAY;
            break;
        case 4:
            board->tiles[i].color = BEIGE;
            break;
        case 5:
            board->tiles[i].color = BROWN;
            break;
        case 6:
            board->tiles[i].color = ORANGE;
            break;
        case 7:
            board->tiles[i].color = RED;
            break;
        case 8: 
            board->tiles[i].color = BLUE;
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

            renderer.test_drawGameBoard(*board);
            //renderer.test_DrawRectangle(board[0]);


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

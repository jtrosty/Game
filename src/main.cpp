//#pragma once
#include "Platform.h"
#include "../include/raylib.h"
#include "render.h"

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800

#define MOUSE_SCALE_MARK_SIZE 12
#define BOARD_WIDTH 5
#define BOARD_HEIGHT 8 


int main(void) {
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;
    int border = 100;
    int numOfTiles = BOARD_HEIGHT * BOARD_WIDTH;

    Platform platform(screenWidth, screenHeight, "Trosty Games", 30);
    Render renderer;

    //TODO: Deletet when done with testing
    Vector2 ballPosition = { 100.0f, 100.0f };
    Color ballColor = DARKBLUE;

    Rectangle rec = {200, 200, 200, 80};
    Vector2 mousePosition = { 0 };
    //////////////////////
    
    // Game Data:
    // Game board test
    //GameData* board = new GameData[40];
    BoardData board {0};
    Tile tiles[BOARD_HEIGHT * BOARD_WIDTH] {0};
    board.tiles = tiles;
    board.numRows = 8;
    board.numCols = 5;
    board.offset = border / 2;
    board.pixelHeight = (screenHeight - border) / board.numRows;
    board.pixelWidth = (screenWidth - border) / board.numCols;

    UnitData heroUnits[(BOARD_HEIGHT * BOARD_WIDTH) / 2] {0};
    UnitData villian[(BOARD_HEIGHT * BOARD_WIDTH) / 2] {0};

    UnitData drowInf;
    drowInf.row = MAX_CHAR;
    drowInf.col = MAX_CHAR;
    drowInf.title = "Drow Infantry";
    drowInf.commander = "Jon";
    drowInf.tier = "02";
    drowInf.dmg = "01";
    drowInf.numOfAtk = "01";
    drowInf.size = "06";
    drowInf.atk = "04";
    drowInf.def = "12";
    drowInf.pow = "03";
    drowInf.toughness = "13";
    drowInf.morale = "01";
    drowInf.communicaiton = "02";

    // Initialize the Board Data
    renderer.resetPlayArea(board);

    //DEBUG: Test code.
    board.tiles[20].hasUnit = 1;
    board.tiles[20].unit = &drowInf;


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
        
        // ----------------------------------------------
        mousePosition = GetMousePosition();
        for (int i = 0; i < numOfTiles; i++) {
            if (board.tiles[i].hasUnit == 0) {

            }
            else {
                // Draw glow around hoverred over item.
                   
                if (CheckCollisionPointRec(mousePosition, board.tiles[i].rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    ballColor = PURPLE;
                }
                else if (CheckCollisionPointRec(mousePosition, board.tiles[i].rec)) {
                    ballColor = YELLOW;
                }
            }
        }

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

            renderer.test_drawGameBoard(board);
            for (int i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++) {
                if (board.tiles[i].hasUnit) {
                    renderer.drawUnitCard(board.tiles[i]);
                }
            }
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

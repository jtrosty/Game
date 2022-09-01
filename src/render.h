#pragma once
#include "../include/raylib.h"
#include "data.h"

class Render {

    public:
    Render();
    ~Render();

    // test Code
    //------------------------------
    void test_DrawRectangle(GameData& gameRec);
    void test_drawGameBoard(GameData bkgdRec[], int width, int height, int row, int col, int offset);

};
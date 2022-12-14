#pragma once
#include "../include/raylib.h"
#include "data.h"

class Render {

    public:
    Render();
    ~Render();

    // test Code
    //------------------------------
    void test_DrawRectangle(BoardGameData& gameRec);
    void test_drawGameBoard(BoardGameData bkgdRec[]);

};
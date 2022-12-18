#pragma once
#include <cassert>
#include "../include/raylib.h"
#include "data.h"


class Render {

    public:
    Render();
    ~Render();

    // test Code
    //------------------------------
    //void test_DrawRectangle(BoardGameData& gameRec);
    void test_drawGameBoard(BoardData& board);
    void drawGameBoard(BoardData& board);
    void drawUnitCard(Tile& tile);
    void drawUnitCard(UnitData& card, Tile& tile);
    void resetPlayArea(BoardData& board);

};
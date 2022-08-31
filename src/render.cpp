#include "render.h"

Render::Render() {}

Render::~Render() {}

void Render::test_DrawRectangle(GameData& gameRec) {
    // Need a piece of data that is updated and feeds into this.
    DrawRectangle(gameRec.x, gameRec.y, gameRec.width, gameRec.height, gameRec.color);
}

void Render::test_drawGameBoard(GameData bkgdRec[], int width, int height, int row, int col) { 
    for (int i = 0; i < 40; i++) {
        //test_drawGameBoard(&(bkgdRec[0]));
        DrawRectangle(bkgdRec[i].x, bkgdRec[i].y, bkgdRec[i].width, bkgdRec[i].height, bkgdRec[i].color);
    }
    for (int i = 0; i <= row; i++) {
        DrawLine(i * row * height, 0, i * row * height, col * width, BLACK);
    }
    for (int i = 0; i <= col; i++) {
        DrawLine(0, i * col * width,  row * height, i * col * width, BLACK);
    }
}
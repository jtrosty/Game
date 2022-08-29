#include "render.h"

Render::Render() {}

Render::~Render() {}

void Render::test_DrawRectangle(GameData& gameRec) {
    // Need a piece of data that is updated and feeds into this.
    DrawRectangle(gameRec.x, gameRec.y, gameRec.width, gameRec.height, gameRec.color);
}

void Render::test_drawGameBoard(GameData bkgdRec[]) { 
    for (int i = 0; i < 39; i++) {
        test_drawGameBoard(&bkgdRec[i]);
    }
}
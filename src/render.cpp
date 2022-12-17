#include "render.h"

Render::Render() {}

Render::~Render() {}

/*
void Render::test_DrawRectangle(BoardData& gameRec) {
    // Need a piece of data that is updated and feeds into this.
    DrawRectangle(gameRec.x, gameRec.y, gameRec.width, gameRec.height, gameRec.color);
}
*/

void Render::test_drawGameBoard(BoardData& board) { 
    Tile temp;
    for (int i = 0; i < 40; i++) {
        temp = board.tiles[i];
        DrawRectangle(temp.x, temp.y, board.pixelWidth, board.pixelHeight, temp.color);
    }
    for (int i = 0; i <= board.numRows; i++) {
        DrawLine(i * board.pixelWidth + board.offset, board.offset, i * board.pixelWidth + board.offset, board.offset + board.numRows * board.pixelHeight, BLACK);
    }
    for (int i = 0; i <= board.numCols; i++) {
        DrawLine( board.offset, board.offset + (i * board.pixelHeight),  board.offset + (board.numCols * board.pixelWidth), board.offset + (i * board.pixelHeight), BLACK);
    }
}
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
        DrawRectangle(temp.rec.x, temp.rec.y, board.pixelWidth, board.pixelHeight, temp.color);
    }
    for (int i = 0; i <= board.numCols; i++) {
        DrawLine(i * board.pixelWidth + board.offset, board.offset, i * board.pixelWidth + board.offset, board.offset + board.numRows * board.pixelHeight, BLACK);
    }
    for (int i = 0; i <= board.numRows; i++) {
        DrawLine( board.offset, board.offset + (i * board.pixelHeight),  board.offset + (board.numCols * board.pixelWidth), board.offset + (i * board.pixelHeight), BLACK);
    }
}

void Render::drawUnitCard(Tile& tile) {
    int offset = 10;
    assert(tile.hasUnit == 1);
    UnitData u = *(tile.unit);
    DrawRectangleRoundedLines(tile.rec, 2.0, 2, 2.0, BLACK);
    DrawText(u.title, tile.rec.x + offset, tile.rec.y, 5, BLACK);
}

void Render::drawUnitCard(UnitData& card, Tile& tile) {

    assert(tile.hasUnit == 1);
    DrawRectangleRoundedLines(tile.rec, 5.0, 2, 2.0, BLACK);

}

void Render::resetPlayArea(BoardData& board) {
    for (int i = 0; i < (board.numCols * board.numRows); i++) {
        board.tiles[i].rec.x = board.offset + board.pixelWidth * (i % board.numCols); 
        board.tiles[i].rec.y = board.offset + board.pixelHeight * (i / board.numCols); 
        board.tiles[i].rec.width = board.pixelWidth;
        board.tiles[i].rec.height = board.pixelHeight;
        board.tiles[i].hasUnit = 0;

        switch (i / board.numCols) 
        {
        case 0:
            board.tiles[i].color = BLUE;
            break;
        case 1:
            board.tiles[i].color = GREEN;
            break;
        case 2:
            board.tiles[i].color = SKYBLUE;
            break;
        case 3:
            board.tiles[i].color = LIGHTGRAY;
            break;
        case 4:
            board.tiles[i].color = BEIGE;
            break;
        case 5:
            board.tiles[i].color = BROWN;
            break;
        case 6:
            board.tiles[i].color = ORANGE;
            break;
        case 7:
            board.tiles[i].color = RED;
            break;
        case 8: 
            board.tiles[i].color = BLUE;
            break;
        default:
            break;
        }
    }
}
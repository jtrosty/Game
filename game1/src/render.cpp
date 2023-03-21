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

void Render::drawUnitCardMouseOver(Tile& tile) {
    drawUnitCard(tile);
    DrawRectangleRoundedLines(tile.rec, 2.0, 2, 2.0, WHITE);
}

void Render::drawUnitCardSelected(Tile& tile) {
    drawUnitCard(tile);
    DrawRectangleRoundedLines(tile.rec, 2.0, 2, 2.0, GREEN);
}

void Render::drawUnitCard(Tile& tile) {
    int offset = 30;
    int spacing25 = 25;
    int spacing = 15;
    int vertSpacing = 10;
    int cardFontSize = 5;
    assert(tile.hasUnit == 1);
    UnitData u = *(tile.unit);
    DrawRectangleRoundedLines(tile.rec, 2.0, 2, 2.0, BLACK);

    int xPos = tile.rec.x + offset;
    int yPos =  tile.rec.y + cardFontSize;
    DrawText(u.title,           xPos,    yPos, cardFontSize, BLACK);
    yPos += vertSpacing;
    DrawText(u.commander,       xPos,    yPos, cardFontSize, BLACK);
    xPos = tile.rec.x + offset;
    yPos += vertSpacing;
    DrawText("SIZE: ",          xPos,    yPos, cardFontSize, BLACK);
    xPos += offset;
    DrawText(u.size,            xPos,    yPos, cardFontSize, BLACK);
    xPos = tile.rec.x + offset;
    yPos += spacing;
    DrawText("ATK: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing25;
    DrawText(u.atk,             xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing;
    DrawText("DEF: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing25;
    DrawText(u.def,             xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing;
    DrawText("POW: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing25;
    DrawText(u.pow,             xPos,    yPos, cardFontSize, BLACK);
    xPos = tile.rec.x + offset;
    yPos += vertSpacing;
    DrawText("TOU: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing25;
    DrawText(u.toughness,       xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing;
    DrawText("MOR: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing25;
    DrawText(u.morale,          xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing;
    DrawText("COM: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing25;
    DrawText(u.communicaiton,   xPos,    yPos, cardFontSize, BLACK);
    xPos = tile.rec.x + offset;
    yPos += vertSpacing;
    DrawText("Num Attacks: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += offset + offset + spacing;
    DrawText(u.numOfAtk,       xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing;
    DrawText("DMG: ",           xPos,    yPos, cardFontSize, BLACK);
    xPos += spacing25;
    DrawText(u.dmg,          xPos,    yPos, cardFontSize, BLACK);
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
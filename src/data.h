#include "../include/raylib.h"
const char MAX_CHAR = 255;

struct UnitData {
    unsigned char row;
    unsigned char col;
    char* title;
    char* commander;
    char* tier;
    char* dmg;
    char* numOfAtk;
    char* size;
    char* atk;
    char* def;
    char* pow;
    char* toughness;
    char* morale;
    char* communicaiton;
    char* traits[5]{0};
};

struct Tile {
    Rectangle rec;
    Color color;
    char hasUnit;
    UnitData* unit;
};

struct BoardData {
    unsigned char numRows;
    unsigned char numCols;
    int offset;
    int pixelWidth;
    int pixelHeight;
    Tile* tiles;
};

struct UserSelection {
    Tile* tileSelected;
    char tileIndex;
    char mouseHeld;
};


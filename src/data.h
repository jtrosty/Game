#include "../include/raylib.h"
const char MAX_CHAR = 255;

struct UnitData {
    unsigned char row;
    unsigned char col;
    char* title;
    char* commander;
    unsigned char tier;
    unsigned char dmg;
    unsigned char size;
    unsigned char atk;
    unsigned char def;
    unsigned char pow;
    unsigned char toughness;
    unsigned char morale;
    unsigned char communicaiton;
    unsigned char traits[5]{0};
};

struct Tile {
    Rectangle rec;
    Color color;
    char hasUnit;
    UnitData* unit;
};

struct BoardData{
    unsigned char numRows;
    unsigned char numCols;
    int offset;
    int pixelWidth;
    int pixelHeight;
    Tile* tiles;
};


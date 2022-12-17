#include "../include/raylib.h"

struct Tile {
    int x;
    int y;
    Color color;
    char hasUnit;
};

struct BoardData{
    unsigned char numRows;
    unsigned char numCols;
    int offset;
    int pixelWidth;
    int pixelHeight;
    Tile* tiles;
};

struct UnitData {
    unsigned char row;
    unsigned char col;
    char title[20];
    char commander[20];
    unsigned char tier;
    unsigned char dmg;
    unsigned char size;
    unsigned char atk;
    unsigned char pow;
    unsigned char toughness;
    unsigned char morale;
    unsigned char communicaiton;
    unsigned char traits[5];
};


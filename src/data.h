#include "../include/raylib.h"

struct BoardGameData {
    int x;
    int y;
    int width;
    int height;
    Color color;
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


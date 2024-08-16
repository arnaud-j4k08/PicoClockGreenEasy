#pragma once

#include <cstdint>

static const int MAX_CHAR_HEIGHT = 7;

enum SpecialCharacter
{
    FixedWidthSpace = 1
};

struct Character
{
    char c;
    uint8_t pixels[MAX_CHAR_HEIGHT];
};

struct ProportionalCharacter
{
    char c;
    int width;
    uint8_t pixels[MAX_CHAR_HEIGHT];
};

struct Font
{
    const int width;
    const int height;
    const Character *chars;
    const ProportionalCharacter *propChars;
};

extern Font narrowFont;
extern Font ultraNarrowFont;
extern Font shortFont;
extern Font classicFont;
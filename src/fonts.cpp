#include "fonts.h"

// clang-format off

static ProportionalCharacter narrowFontChars[] =
{
    {FixedWidthSpace, 3,
             {  0b000,
                0b000,
                0b000,
                0b000,
                0b000,
                0b000,
                0b000 } },

    {' ', 1, {  0b0,
                0b0,
                0b0,
                0b0,
                0b0,
                0b0,
                0b0 } },

    {'%', 3, {  0b101,
                0b001,
                0b010,
                0b010,
                0b010,
                0b100,
                0b101 }},

    {'(', 2, {  0b01,
                0b10,
                0b10,
                0b10,
                0b10,
                0b10,
                0b01 } },

    {')', 2, {  0b10,
                0b01,
                0b01,
                0b01,
                0b01,
                0b01,
                0b10 } },

    {'-', 2, {  0b00,
                0b00,
                0b00,
                0b11,
                0b00,
                0b00,
                0b00 } },

    {'0', 3, {  0b010,
                0b101,
                0b101,
                0b101,
                0b101,
                0b101,
                0b010 } },

    {'1', 3, {  0b001,
                0b011,
                0b001,
                0b001,
                0b001,
                0b001,
                0b001 } },

    {'2', 3, {  0b010,
                0b101,
                0b001,
                0b010,
                0b100,
                0b100,
                0b111 } },

    {'3', 3, {  0b010,
                0b101,
                0b001,
                0b010,
                0b001,
                0b101,
                0b010 } },

    {'4', 3, {  0b001,
                0b011,
                0b101,
                0b111,
                0b001,
                0b001,
                0b001 } },

    {'5', 3, {  0b111,
                0b100,
                0b100,
                0b110,
                0b001,
                0b001,
                0b110 } },

    {'6', 3, {  0b010,
                0b101,
                0b100,
                0b110,
                0b101,
                0b101,
                0b010 } },

    {'7', 3, {  0b111,
                0b001,
                0b010,
                0b010,
                0b010,
                0b010,
                0b010 } },

    {'8', 3, {  0b010,
                0b101,
                0b101,
                0b010,
                0b101,
                0b101,
                0b010 } },

    {'9', 3, {  0b010,
                0b101,
                0b101,
                0b011,
                0b001,
                0b101,
                0b010 } },

    {':', 1, {  0b0,
                0b0,
                0b1,
                0b0,
                0b1,
                0b0,
                0b0 } },

    {'A', 3, {  0b010,
                0b101,
                0b101,
                0b111,
                0b101,
                0b101,
                0b101 } },

    {'B', 3, {  0b110,
                0b101,
                0b101,
                0b110,
                0b101,
                0b101,
                0b110 } },

    {'C', 3, {  0b010,
                0b101,
                0b100,
                0b100,
                0b100,
                0b101,
                0b010 } },

    {'D', 3, {  0b110,
                0b101,
                0b101,
                0b101,
                0b101,
                0b101,
                0b110 } },

    {'E', 2, {  0b11,
                0b10,
                0b10,
                0b11,
                0b10,
                0b10,
                0b11 } },

    {'F', 2, {  0b11,
                0b10,
                0b10,
                0b11,
                0b10,
                0b10,
                0b10 } },

    {'G', 3, {  0b011,
                0b100,
                0b100,
                0b101,
                0b101,
                0b101,
                0b010 } },

    {'H', 3, {  0b101,
                0b101,
                0b101,
                0b111,
                0b101,
                0b101,
                0b101 } },

    {'I', 1, {  0b1,
                0b1,
                0b1,
                0b1,
                0b1,
                0b1,
                0b1 } },

    {'J', 3, {  0b001,
                0b001,
                0b001,
                0b001,
                0b101,
                0b101,
                0b010 } },

    {'K', 3, {  0b101,
                0b101,
                0b101,
                0b110,
                0b101,
                0b101,
                0b101 } },

    {'L', 2, {  0b10,
                0b10,
                0b10,
                0b10,
                0b10,
                0b10,
                0b11 } },

    {'M', 5, {  0b10001,
                0b11011,
                0b10101,
                0b10001,
                0b10001,
                0b10001,
                0b10001 } },

    {'N', 4, {  0b1001,
                0b1001,
                0b1101,
                0b1011,
                0b1001,
                0b1001,
                0b1001 } },

    {'O', 3, {  0b010,
                0b101,
                0b101,
                0b101,
                0b101,
                0b101,
                0b010 } },

    {'P', 3, {  0b110,
                0b101,
                0b101,
                0b110,
                0b100,
                0b100,
                0b100 } },

    {'Q', 4, {  0b0110,
                0b1001,
                0b1001,
                0b1001,
                0b1001,
                0b1010,
                0b0101 } },

    {'R', 3, {  0b110,
                0b101,
                0b101,
                0b110,
                0b101,
                0b101,
                0b101 } },

    {'S', 3, {  0b011,
                0b100,
                0b100,
                0b010,
                0b001,
                0b001,
                0b110 } },

    {'T', 3, {  0b111,
                0b010,
                0b010,
                0b010,
                0b010,
                0b010,
                0b010 } },

    {'U', 3, {  0b101,
                0b101,
                0b101,
                0b101,
                0b101,
                0b101,
                0b010 } },

    {'V', 3, {  0b101,
                0b101,
                0b101,
                0b101,
                0b101,
                0b010,
                0b010 } },

    {'W', 5, {  0b10001,
                0b10001,
                0b10001,
                0b10101,
                0b10101,
                0b01010,
                0b01010 } },

    {'X', 3, {  0b101,
                0b101,
                0b101,
                0b010,
                0b101,
                0b101,
                0b101 } },

    {'Y', 3, {  0b101,
                0b101,
                0b101,
                0b010,
                0b010,
                0b010,
                0b010 } },

    {'Z', 3, {  0b111,
                0b001,
                0b001,
                0b010,
                0b100,
                0b100,
                0b111 } },

    {0, {}} // End
};

Font narrowFont = {3, 7, nullptr, narrowFontChars};

static Character ultraNarrowChars[] =
{
    {'1', { 0b01,
            0b11,
            0b01,
            0b01,
            0b01,
            0b01,
            0b01 } },

    {'2', { 0b10,
            0b01,
            0b01,
            0b11,
            0b10,
            0b10,
            0b11 } },
    {0, {}} // Indicate the end of the font definition
};

Font ultraNarrowFont = {2, 7, ultraNarrowChars};

static Character shortChars[] =
{
  {'0', { 0b0110,
          0b1001,
          0b1001,
          0b1001,
          0b0110 } },

  {'1', { 0b0010,
          0b0110,
          0b0010,
          0b0010,
          0b0111 } },

  {'2', { 0b0110,
          0b1001,
          0b0010,
          0b0100,
          0b1111 } },

  {'3', { 0b1110,
          0b0001,
          0b0110,
          0b0001,
          0b1110 } },

  {'4', { 0b0010,
          0b0110,
          0b1010,
          0b1111,
          0b0010 } },

  {'5', { 0b1111,
          0b1000,
          0b1110,
          0b0001,
          0b1110 } },

  {'6', { 0b0111,
          0b1000,
          0b1110,
          0b1001,
          0b0110 } },

  {'7', { 0b1111,
          0b0001,
          0b0010,
          0b0100,
          0b0100 } },

  {'8', { 0b0110,
          0b1001,
          0b0110,
          0b1001,
          0b0110 } },

  {'9', { 0b0110,
          0b1001,
          0b0111,
          0b0001,
          0b1110 } },

    {0, {}} // Indicate the end of the font definition
};

Font shortFont = {4, 5, shortChars};

static Character classicChars[] =
{
  {' ', { 0,
          0,
          0,
          0,
          0,
          0,
          0 } },

  {'0', { 0b0110,
          0b1001,
          0b1001,
          0b1001,
          0b1001,
          0b1001,
          0b0110 } },

  {'1', { 0b0010,
          0b0110,
          0b0010,
          0b0010,
          0b0010,
          0b0010,
          0b0111 } },

  {'2', { 0b0110,
          0b1001,
          0b0001,
          0b0010,
          0b0100,
          0b1000,
          0b1111 } },

  {'3', { 0b1110,
          0b0001,
          0b0001,
          0b0110,
          0b0001,
          0b0001,
          0b1110 } },

  {'4', { 0b0010,
          0b0110,
          0b1010,
          0b1111,
          0b0010,
          0b0010,
          0b0010 } },

  {'5', { 0b1111,
          0b1000,
          0b1000,
          0b1110,
          0b0001,
          0b0001,
          0b1110 } },

  {'6', { 0b0111,
          0b1000,
          0b1000,
          0b1110,
          0b1001,
          0b1001,
          0b0110 } },

  {'7', { 0b1111,
          0b0001,
          0b0010,
          0b0100,
          0b0100,
          0b0100,
          0b0100 } },

  {'8', { 0b0110,
          0b1001,
          0b1001,
          0b0110,
          0b1001,
          0b1001,
          0b0110 } },

  {'9', { 0b0110,
          0b1001,
          0b1001,
          0b0111,
          0b0001,
          0b0001,
          0b1110 } },

    {0, {}} // End
};

Font classicFont = {4, 7, classicChars};
#pragma once

#include "fonts.h"
#include "PicoClockHw/Display.h"

#include <string>

class Bitmap
{
public:
    static const int HEIGHT = Display::HEIGHT;
    static const int WIDTH = Display::WIDTH;

    enum Indicator
    {
        MoveOn = 0,
        AlarmOn = 1,
        CountDown = 2,
        F = 3,
        C = -3,
        Am = 4,
        Pm = -4,
        CountUp = 5,
        Hourly = 6,
        AutoLight = 7
    };

    Bitmap();

    void clear();
    const uint32_t *buffer() const { return m_frameBuffer; }
    void setDrawOrigin(int x, int y);

    void setFont(const Font *font);
    bool pixel(int x, int y) const; // Does not consider the draw origin
    void putPixel(int x, int y, bool on);
    void drawRectangle(int left, int top, int right, int bottom, bool on);
    void moveRectangle(int left, int top, int right, int bottom, int vertShift);
    void copyRectangle(int left, int top, int right, int bottom, Bitmap &destBmp, int destTop);
    int drawChar(int x, int y, char c); // Return the width of the char
    int charWidth(char c) const;
    void draw2DigitsInt(int x, int y, int i);
    void draw2DigitsIntWithLeadingZero(int x, int y, int i);
    int drawText(int x, int y, const std::string &s); // Return the width of the text
    int textWidth(const std::string &s) const;

    // weekDay is in days since Sunday, like in the tm structure
    void putWeekDay(int weekDay, bool on);
    
    void putWeekDays(uint8_t weekDayBits);

    void putIndicator(Indicator i, bool on);
    void drawMiddleDots();

private:
    void considerDrawOrigin(int &x, int &y);
    void unconsiderDrawOrigin(int &x, int &y);

    uint32_t m_frameBuffer[HEIGHT];
    int m_drawOriginX, m_drawOriginY;
    const Font *m_currentFont = nullptr;
};

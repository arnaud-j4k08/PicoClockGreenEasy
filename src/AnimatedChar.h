#pragma once

#include "Bitmap.h"

#include "PicoClockHw/Buzzer.h"

class AnimatedChar
{
public:
    AnimatedChar(int x, int y) : m_x(x), m_y(y) {}
    void renderChar(Bitmap &frame, bool fullRefresh, char c);

    static void render2DigitsInt(
        AnimatedChar &digit1, AnimatedChar &digit2, Bitmap &frame, bool fullRefresh, int i);
    static void render2DigitsIntWithLeadingZero(
        AnimatedChar &digit1, AnimatedChar &digit2, Bitmap &frame, bool fullRefresh, int i);

private:
    void pixelCoordAtRank(Bitmap &frame, int &x, int &y, int rank);
    bool tryToMovePixel(Bitmap &frame, int x, int y);

    int m_x, m_y; // Position of the character in the frame
    char m_targetChar = 0;
    Bitmap m_targetCharBitmap;
    CyclicCounter m_frameCounter{Display::FRAME_RATE / 10};
    bool m_transitioning = false;
};

            

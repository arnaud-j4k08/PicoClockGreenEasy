#include "AnimatedChar.h"
#include "Bitmap.h"
#include "PicoClockHw/Platform.h"
#include "Utils/Trace.h"
#include <bitset>

void AnimatedChar::render2DigitsInt(
    AnimatedChar &digit1, AnimatedChar &digit2, Bitmap &frame, bool fullRefresh, int i)
{
    digit1.renderChar(frame, fullRefresh, i < 10 ? ' ' : '0' + i / 10);
    digit2.renderChar(frame, fullRefresh, '0' + i % 10);
}

void AnimatedChar::render2DigitsIntWithLeadingZero(
    AnimatedChar &digit1, AnimatedChar &digit2, Bitmap &frame, bool fullRefresh, int i)
{
    digit1.renderChar(frame, fullRefresh, '0' + i / 10);
    digit2.renderChar(frame, fullRefresh, '0' + i % 10);
}

void AnimatedChar::renderChar(Bitmap &frame, bool fullRefresh, char c)
{
    if (m_targetChar != c)
    {
        TRACE  <<"Change target char from" << m_targetChar << "to" << c;
        m_targetChar = c;
        m_targetCharBitmap.clear();
        m_targetCharBitmap.setFont(&classicFont);
        m_targetCharBitmap.drawChar(0, 0, c);
        m_transitioning = true;
    }

    if (fullRefresh)
        m_transitioning = true;

    if (!m_transitioning)
        return;

    if (!m_frameCounter.increment())
        return; // Do not render the character yet to slow down the animation

    // Count how many pixels are currently on for the current character.
    int pixelCount = 0;
    for (int y = 0; y < classicFont.height; ++y)
    {
        for (int x = 0; x < classicFont.width; ++x)
        {
            if (frame.pixel(m_x + x, m_y + y))
                pixelCount++;
        }
    }

    if (pixelCount == 0)
    {
        if (m_targetChar == ' ')
        {
            // Do not draw anything if the target char is a space and stop transitioning, as there
            // are already no pixels.
            m_transitioning = false;
            return; 
        }
        TRACE << "Draw a dot in the middle if no pixels";
        frame.putPixel(
            m_x + classicFont.width / 2, m_y + classicFont.height / 2, true);
    } 
    else
    {
        // Track moveable pixel in a bitset, to be able to detect when there will be no moveable 
        // ones anymore.
        std::bitset<28> moveablePixels; // one bit for each pixel of a character of the classicFont
        for (int i = 0; i < pixelCount; i++)
            moveablePixels.set(i);

        // Loop until a pixel to move is found or all pixels have been moved.
        while (moveablePixels.any())
        {
            int pixelRank = Platform::randomNumber32() % pixelCount;

            if (!(moveablePixels.test(pixelRank)))
                continue; // Skip already checked pixels

            int x, y;
            pixelCoordAtRank(frame, x, y, pixelRank);

            // Leave the loop if a pixel has been moved.
            if (tryToMovePixel(frame, x, y))
            {
                TRACE << "Moved pixel at" << x << y;
                break;
            }
            else
            {
                moveablePixels.reset(pixelRank); // Remove this pixel from the moveable pixels
            }
        }

        if (moveablePixels.none())
        {
            TRACE << "All pixels moved";
            
            // Since all pixels have been moved, stop transitioning.
            m_transitioning = false;
        }
    }
}

bool AnimatedChar::tryToMovePixel(Bitmap &frame, int x, int y)
{
    // Adopt the state of this pixel in the target character.
    bool moved = false; 
    if (frame.pixel(m_x + x, m_y + y) != m_targetCharBitmap.pixel(x, y))
    {
        frame.putPixel(m_x + x, m_y + y, m_targetCharBitmap.pixel(x, y));
        moved = true;
    }
            
    // Check if the state of the pixel can also be moved to a neighbor pixel. If so, move it.
    struct Coord
    {
        int x, y;
    };
    Coord neighbors[] = {
        {-1, 0}, {0, -1}, {0, 1}, {1, 0}, 
        {1, 1}, {-1, -1}, {-1, 1},{1, -1},
    };
    static const int NEIGHBORS_COUNT = sizeof(neighbors) / sizeof(Coord);
    std::bitset<NEIGHBORS_COUNT> neighborsToCheck;
    for (int i = 0; i < NEIGHBORS_COUNT; i++)
        neighborsToCheck.set(i);
    while (neighborsToCheck.any())
    {
        int i = Platform::randomNumber32() % NEIGHBORS_COUNT;

        if (!neighborsToCheck.test(i))
            continue; // Skip already checked neighbors

        const Coord &n = neighbors[i];

        if (x + n.x < 0 || x + n.x >= classicFont.width ||
            y + n.y < 0 || y + n.y >= classicFont.height)
        {
            neighborsToCheck.reset(i);
            continue; // Skip out of bounds neighbors
        }

        if (m_targetCharBitmap.pixel(x + n.x, y + n.y) &&
            !frame.pixel(m_x + x + n.x, m_y + y + n.y))
        {
            frame.putPixel(m_x + x + n.x, m_y + y + n.y, true);
            moved = true;
            break;
        } else
            neighborsToCheck.reset(i);
    }

    return moved;
}

void AnimatedChar::pixelCoordAtRank(Bitmap &frame, int &x, int &y, int pixelRank)
{
    for (y = 0; y < classicFont.height; ++y)
    {
        for (x = 0; x < classicFont.width; ++x)
        {
            if (frame.pixel(m_x + x, m_y + y))
            {
                if (pixelRank == 0)
                    return;
                pixelRank--;
            }
        }
    }
}

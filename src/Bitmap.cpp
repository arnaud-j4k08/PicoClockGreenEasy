#include "Bitmap.h"
#include "Utils/Trace.h"

#include <map>

namespace
{
    struct SortedFont
    {
        std::map<char, const Character *> charsIndex;
        std::map<char, const ProportionalCharacter *> propCharsIndex;
    };  

    std::map<const Font *, SortedFont> g_sortedFonts;

    template <typename CharType>
    void buildIndex(const CharType *chars, std::map<char, const CharType *> &charsIndex)
    {
        if (charsIndex.empty())
        {
            while (chars->c != 0)
            {
                charsIndex.insert(std::make_pair(chars->c, chars));
                chars++;
            }
        }
    }

    uint32_t rowMask(int left, int right)
    {
        return (0xFFFFFFFF >> left) & (0xFFFFFFFF << (31 - right));
    }
}

Bitmap::Bitmap()
{
    clear();
}

void Bitmap::setDrawOrigin(int x, int y)
{
    m_drawOriginX = x;
    m_drawOriginY = y;
}

void Bitmap::considerDrawOrigin(int &x, int &y)
{
    x += m_drawOriginX;
    y += m_drawOriginY;
}

void Bitmap::unconsiderDrawOrigin(int &x, int &y)
{
    x -= m_drawOriginX;
    y -= m_drawOriginY;
}

bool Bitmap::pixel(int x, int y) const
{
    uint32_t mask = 1 << (31 - x%32);

    return m_frameBuffer[y] & mask;
}

void Bitmap::putPixel(int x, int y, bool on)
{
    considerDrawOrigin(x, y);

    if (x < 0 || y < 0 || x >= Display::WIDTH || y >= Display::HEIGHT)
        return;

    uint32_t mask = 1 << (31 - x%32);
    if (on)
        m_frameBuffer[y] |= mask;
    else
        m_frameBuffer[y] &= ~mask;
}

void Bitmap::putIndicator(Indicator i, bool on)
{
    uint32_t mask;
    int row = i;
    switch(i)
    {
        case F:
        case Am:
            // Indicator on left led
            mask = 1 << 31;
            break;
        case C:
        case Pm:
            // Indicator on right led
            mask = 1 << 30;
            
            // The row number is the negative of the value for C and PM.
            row = -i; 
            break;
        default:
            // All other indicators use 2 leds.
            mask = (1 << 31) | (1 << 30);
            break;
    }

    if (on)
        m_frameBuffer[row] |= mask;
    else
        m_frameBuffer[row] &= ~mask;
}

void Bitmap::drawRectangle(int left, int top, int right, int bottom, bool on)
{
    considerDrawOrigin(left, top);
    considerDrawOrigin(right, bottom);

    uint32_t mask = rowMask(left, right);
    for (int y = top; y <= bottom;y++)
    {
        if (on)
            m_frameBuffer[y] |= mask;
        else
            m_frameBuffer[y] &= ~mask;
    }
}

void Bitmap::moveRectangle(int left, int top, int right, int bottom, int vertShift)
{
    considerDrawOrigin(left, top);
    considerDrawOrigin(right, bottom);

    uint32_t mask = rowMask(left, right);
    if (vertShift > 0)
    {
        for (int y = bottom; y >= top; y--)
        {
            m_frameBuffer[y + vertShift] &= ~mask;
            m_frameBuffer[y + vertShift] |= m_frameBuffer[y] & mask;
        }
    }
    else if (vertShift < 0)
    {
        TRACE << "Move row" << top << "to" << bottom;
        for (int y = top; y <= bottom; y++)
        {
            m_frameBuffer[y + vertShift] &= ~mask;
            m_frameBuffer[y + vertShift] |= m_frameBuffer[y] & mask;
        }
    }
}

void Bitmap::copyRectangle(int left, int top, int right, int bottom, Bitmap &destBmp, int destTop)
{
    considerDrawOrigin(left, top);
    considerDrawOrigin(right, bottom);
    int dummy;
    destBmp.considerDrawOrigin(dummy, destTop);

    uint32_t mask = rowMask(left, right);
    for (int y = top; y <= bottom; y++)
    {
        destBmp.m_frameBuffer[destTop] &= ~mask;
        destBmp.m_frameBuffer[destTop] |= m_frameBuffer[y] & mask;
        destTop++;
    }
}

void Bitmap::clear()
{
    for(int i=0; i< Display::HEIGHT;i++)
    {
        m_frameBuffer[i] = 0;
    }
}

void Bitmap::setFont(const Font *font) 
{ 
    m_currentFont = font; 

    SortedFont &sortedFont = g_sortedFonts[m_currentFont];
    buildIndex(m_currentFont->chars, sortedFont.charsIndex);
    buildIndex(m_currentFont->propChars, sortedFont.propCharsIndex);
}

int Bitmap::drawChar(int x, int y, char c)
{
    considerDrawOrigin(x, y);

    // Currently, fonts support only capital letters.
    c = toupper(c);

    SortedFont &sortedFont = g_sortedFonts.at(m_currentFont);
    auto itCharDef = sortedFont.charsIndex.find(c);
    if (itCharDef != sortedFont.charsIndex.end())
    {
        // Draw the char if it is visible.
        if (x + m_currentFont->width > 0 && x < WIDTH)
        {
            for (int i = 0; i < m_currentFont->height; i++)
            {
                m_frameBuffer[y + i] |= itCharDef->second->pixels[i] << (32 - x - m_currentFont->width);
            }
        }
        return m_currentFont->width;
    }

    auto itPropCharDef = sortedFont.propCharsIndex.find(c);
    if (itPropCharDef != sortedFont.propCharsIndex.end())
    {
        // Draw the char if it is visible.
        if (x + itPropCharDef->second->width > 0 && x < WIDTH)
        {
            for (int i = 0; i < m_currentFont->height; i++)
            {
                m_frameBuffer[y + i] |= itPropCharDef->second->pixels[i] << (32 - x - itPropCharDef->second->width);
            }
        }
        return itPropCharDef->second->width;
    }

    // Draw a rectangle to indicate that the char is undefined.
    unconsiderDrawOrigin(x, y);
    drawRectangle(x, y, x + m_currentFont->width - 1, m_currentFont->height - 1, true);
    
    return m_currentFont->width;
}

int Bitmap::charWidth(char c) const
{
    // Currently, fonts support only capital letters.
    c = toupper(c);

    SortedFont &sortedFont = g_sortedFonts.at(m_currentFont);

    auto itPropCharDef = sortedFont.propCharsIndex.find(c);
    if (itPropCharDef != sortedFont.propCharsIndex.end())
    {
        // Proportional char
        return itPropCharDef->second->width;
    } else
    {
        // Fix width or unknown char
        return m_currentFont->width;
    }
}

void Bitmap::draw2DigitsInt(int x, int y, int i)
{
    // No need to call considerDrawOrigin as drawChar which is called below does it

    if (i >= 10) // No leading zero
        drawChar(x, y, '0' + i / 10);

    drawChar(x + m_currentFont->width + 1, y, '0' + i % 10);
}

void Bitmap::draw2DigitsIntWithLeadingZero(int x, int y, int i)
{
    // No need to call considerDrawOrigin as drawChar which is called below does it

    drawChar(x, y, '0' + i / 10);
    drawChar(x + m_currentFont->width + 1, y, '0' + i % 10);
}

int Bitmap::drawText(int x, int y, const std::string &s)
{
    // No need to call considerDrawOrigin as drawChar which is called below does it

    int textWidth = -1; // So that the last spacing is not counted.
    for (char c : s)
    {
        int width = drawChar(x, y, c);
        x += width + 1;
        textWidth += width + 1;
    }

    return textWidth;
}

int Bitmap::textWidth(const std::string &s) const
{
    int textWidth = -1; // So that the last spacing is not counted.

    for (char c : s)
        textWidth += charWidth(c) + 1;


    return textWidth;
}

void Bitmap::putWeekDay(int weekDay, bool on)
{
    // Change draw origin to access indicators
    int saveDrawOriginX = m_drawOriginX;
    int saveDrawOriginY = m_drawOriginY;
    setDrawOrigin(0, 0);

    // Adapt weekDay as the display starts with Monday
    weekDay = weekDay == 0 ? 6 : weekDay - 1;

    // Turn the given week day on (2 leds)
    int x = 3 + weekDay * 3;
    drawRectangle(x, 0, x + 1, 0, on);

    // Restore draw origin
    setDrawOrigin(saveDrawOriginX, saveDrawOriginY);
}

void Bitmap::putWeekDays(uint8_t weekDayBits)
{
    TRACE << "weekDayBits:" << weekDayBits;

    // Change draw origin to access indicators
    int saveDrawOriginX = m_drawOriginX;
    int saveDrawOriginY = m_drawOriginY;
    setDrawOrigin(0, 0);

    // Monday to Saturday
    for (int i = 1; i <= 6; i++)
    {
        int x = i * 3;
        drawRectangle(x, 0, x + 1, 0, weekDayBits & (1 << i));
    }

    // Sunday
    drawRectangle(21, 0, 22, 0, weekDayBits & 1);

    // Restore draw origin
    setDrawOrigin(saveDrawOriginX, saveDrawOriginY);
}

void Bitmap::drawMiddleDots()
{
    drawRectangle(10, 1, 11, 2, true);
    drawRectangle(10, 4, 11, 5, true);
}
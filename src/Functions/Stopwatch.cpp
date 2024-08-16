#include "Stopwatch.h"

void Stopwatch::reset()
{
    m_min = 0;
    m_sec = 0;
    m_tick = 0;

    // Select the stopwatch function, as this method may be called from another function.
    select();
}

void Stopwatch::tick()
{
    if (!m_running)
        return;

    bool wrapped = m_tick.increment();
    if (wrapped)
        wrapped = m_sec.increment();
    if (wrapped)
        m_min.increment();
}

void Stopwatch::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    if (!fullRefresh && !m_running)
        return;

    frame.clear();
    frame.setFont(&classicFont);

    if (m_min == 0)
    {
        // In the first minute, display seconds and centiseconds
        frame.draw2DigitsIntWithLeadingZero(0, 0, m_sec);
        frame.draw2DigitsIntWithLeadingZero(13, 0, m_tick * 100 / Display::FRAME_RATE);
    } else
    {
        // After the first minute, display minutes and seconds
        frame.draw2DigitsIntWithLeadingZero(0, 0, m_min);
        frame.draw2DigitsIntWithLeadingZero(13, 0, m_sec);
    }

    frame.drawMiddleDots();

    frame.putIndicator(Bitmap::CountUp, true);
}

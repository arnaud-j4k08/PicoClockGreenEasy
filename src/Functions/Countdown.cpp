#include "Countdown.h"
#include "Utils/Trace.h"
#include "PicoClockHw/Buzzer.h"

namespace
{
    const int STOP_RINGING_AFTER_SEC = 5 * 60; // Stop ringing after 5 minutes
}

Countdown::Countdown(ClockUi *clockUi, std::vector<std::unique_ptr<AbstractFunction>> *parentMenu) : 
    AbstractFunction(clockUi), m_parentMenu(parentMenu)
{
    initCount();

    TRACE << "m_sec:" << m_sec;
}

void Countdown::tick()
{
    if (m_state == Running)
    {
        // m_tick moves forward, but m_sec and m_min moves backward. This is to make the second 
        // delay pass before the counter is visually decreased.
        bool wrapped = m_tick.increment();
        if (wrapped)
            wrapped = m_sec.decrement();
        if (wrapped)
            m_min--;

        // When 00:00 is reached, the countdown is finished. 
        if (m_sec == 0 && m_min == 0)
        {
            m_state = Ringing;
            m_ringingCounter = 0;
            m_ringingForSecs = 0;

            // Reset the counter to the initial value
            initCount();
            forceRefresh();
            
            // Make the function visible in case the user switched to another function in the 
            // meantime.
            setCurrentMenu(m_parentMenu, this);
        }
    }

    if (m_state == Ringing)
    {
        // Make the buzzer sound twice on every second
        if (m_ringingCounter == 0 || m_ringingCounter == Display::FRAME_RATE / 4)
            buzzer().beepForMs(50);

        if (m_ringingCounter == 0)
        {
            m_ringingForSecs++;
            if (m_ringingForSecs >= STOP_RINGING_AFTER_SEC)
                m_state = Stopped;
        }

        m_ringingCounter.increment();
    } 
}

void Countdown::set()
{
    m_state = Stopped;
    initCount();
    select();
    editValues();
}

bool Countdown::stopRinging()
{
    if (m_state == Ringing)
    {
        m_state = Stopped;
        return true;
    } else
        return false;
}

void Countdown::activate()
{
    if (editedValueIndex() == 0)
    {
        switch(m_state)
        {
            case Stopped:
                m_state = Running;
                break;
            case Running:
                m_state = Stopped;
                break;
        }
    }
    else
        editValues();
}

void Countdown::initCount()
{
    m_min = settings().countdownStartMin;
    m_sec = settings().countdownStartSec;
    m_tick = 0;
}

void Countdown::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    if (editedValueIndex != NoEditing && 
        (blinkingCounter == 0 || blinkingCounter == BLINKING_DISAPPEAR_FRAME))
        fullRefresh = true;

    if (!fullRefresh && m_state != Running)
        return;

    frame.clear();
    frame.setFont(&classicFont);

    if (editedValueIndex != EditingMin || blinkingCounter < BLINKING_DISAPPEAR_FRAME)
        frame.draw2DigitsIntWithLeadingZero(0, 0, m_min);

    if (editedValueIndex != EditingSec || blinkingCounter < BLINKING_DISAPPEAR_FRAME)
        frame.draw2DigitsIntWithLeadingZero(13, 0, m_sec);

    if (m_tick == 0 || m_tick > Display::FRAME_RATE / 2)
        frame.drawMiddleDots();

    frame.putIndicator(Bitmap::CountDown, true);
}

void Countdown::modifyValue(int valueIndex, Direction direction)
{
    switch(valueIndex)
    {
        case EditingMin:
            adjustField(direction, Minute, modifySettings().countdownStartMin);
            break;

        case EditingSec:
            adjustField(direction, Second, modifySettings().countdownStartSec);
            break;
    }

    // Prevent from setting the start to 00:00
    if (settings().countdownStartMin == 0 && settings().countdownStartSec == 0)
    {
        if (valueIndex == EditingMin || direction == Up || direction == RepeatedUp)
            modifySettings().countdownStartSec = 1;
        else 
            modifySettings().countdownStartSec = 59;
    }

    initCount();
}

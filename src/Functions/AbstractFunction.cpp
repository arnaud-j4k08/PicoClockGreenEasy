#include "AbstractFunction.h"
#include "Utils/Trace.h"
#include "Clock.h"
#include "ClockUi.h"

#include <ctime>

namespace 
{
    int daysInMonth(tm dt)
    {
        // Go to next month
        dt.tm_mon++;

        // Go to next year if reached
        if (dt.tm_mon >= 12)
        {
            dt.tm_mon = 0;
            dt.tm_year++;
        }

        // Set month day to 0, which is equivalent to the last day of the previous month
        dt.tm_mday = 0;

        // Call mktime to make a valid date
        mktime(&dt);

        // Now we have the number of days of the given month and can return it
        return dt.tm_mday;
    }
}

const Settings::Values &AbstractFunction::settings() const
{
    return m_clockUi->m_settings.get();
}

Settings::Values &AbstractFunction::modifySettings()
{
    return m_clockUi->m_settings.modify();
}

const Clock &AbstractFunction::clock() const
{
    return m_clockUi->m_clock;
}

Clock &AbstractFunction::clock()
{
    return m_clockUi->m_clock;
}

void AbstractFunction::convertHour(int hour24, int &displayedHour, bool &morning) const
{
    if (settings().format24h)
        displayedHour = hour24;
    else
    {
        morning = hour24 < 12;
        displayedHour =  morning ? hour24 : hour24 - 12;
        
        if (displayedHour == 0)
            displayedHour = 12;
    }
}

void AbstractFunction::putWeekDay(Bitmap &frame)
{
    frame.putWeekDay(clock().get().tm_wday, true);
}

void AbstractFunction::putAmPmIndicators(Bitmap &frame, bool morning)
{
    frame.putIndicator(Bitmap::Am, !settings().format24h && morning);
    frame.putIndicator(Bitmap::Pm, !settings().format24h && !morning);
}

void AbstractFunction::editValues()
{
    m_clockUi->editValues();
}

int AbstractFunction::editedValueIndex() const
{
    return m_clockUi->m_editedValueIndex;
}

void AbstractFunction::adjustField(Field field, Direction dir)
{
    int *value = nullptr;
    int modulo = 0;
    int bigStepSize = 0;
    int floor = 0;
    tm tm = clock().get();
    bool tmTouched = false;
    switch(field)
    {
        case Year:
            value = &tm.tm_year;
            tmTouched = true;
            break;
        case Month:
            value = &tm.tm_mon;
            tmTouched = true;
            break;
        case Day:
            value = &tm.tm_mday;
            tmTouched = true;
            break;
        case Hour:
            value = &tm.tm_hour;
            tmTouched = true;
            break;
        case Minute:
            value = &tm.tm_min;
            tmTouched = true;

            // Changing minutes also resets seconds and ticks to 0. This way, the user can synchronize precisely
            // with another clock.
            tm.tm_sec = 0;
            clock().resetTicks();

            break;
        case Alarm1Hour:
            value = &modifySettings().alarm1.hour;
            break;
        case Alarm1Minute:
            value = &modifySettings().alarm1.min;
            break;
        case Alarm2Hour:
            value = &modifySettings().alarm2.hour;
            break;
        case Alarm2Minute:
            value = &modifySettings().alarm2.min;
            break;
        case CountdownStartMinute:
            value = &modifySettings().countdownStartMin;
            break;
        case CountdownStartSecond:
            value = &modifySettings().countdownStartSec;
            break;
    }
    switch(field)
    {
        case Year:
            modulo = 200;
            floor = 100; // Year 2000, as tm::tm_year is 1900-based
            bigStepSize = 10;
            break;
        case Month:
            modulo = 12;
            bigStepSize = 3;
            break;
        case Day:
            modulo = daysInMonth(tm);
            floor = 1;
            bigStepSize = 7;
            break;
        case Hour:
        case Alarm1Hour:
        case Alarm2Hour:
            modulo = 24;
            bigStepSize = 6;
            break;
        case Minute:
        case Alarm1Minute:
        case Alarm2Minute:
        case CountdownStartMinute:
        case CountdownStartSecond:
            modulo = 60;
            bigStepSize = 10;
            break;
    }

    TRACE << "Previous value:" << *value;
    *value -= floor;
    switch (dir)
    {
        case Up:
            *value = (*value + 1) % modulo;
            break;
        case Down:
            *value = (*value + modulo - 1) % modulo;
            break;
        case RepeatedUp:
            // Round to the big step multiple or move up to the next one
            *value = ((*value + bigStepSize) % modulo) / bigStepSize * bigStepSize;
            break;
        case RepeatedDown:
            // Round to the big step multiple or move down to the next one
            *value = ((*value + modulo - 1) % modulo) / bigStepSize * bigStepSize;
            break;
    }
    *value += floor;
    TRACE << "New value: " << *value;

    if (tmTouched)
    {
        // Save time in the program clock. RTC sync will be triggered when leaving edit mode.
        clock().set(tm);
    }
}

void AbstractFunction::setBlinkingCounter(int counter)
{
    m_clockUi->m_blinkingCounter = counter;
}

void AbstractFunction::forceRefresh()
{
    m_clockUi->m_forceRefresh = true;
}

void AbstractFunction::bringScrollingToRight()
{
    m_clockUi->bringHorizScrollingToRight();
}

void AbstractFunction::renderScrollingText(
    Bitmap &frame,
    bool fullRefresh,
    const std::string &leftText, 
    const std::string &editedValue, 
    const std::string &rightText)
{
    m_clockUi->renderHorizScrollingText(frame, fullRefresh, leftText, editedValue, rightText);
}

void AbstractFunction::setCurrentMenu(
    std::vector<std::unique_ptr<AbstractFunction>> *menu, AbstractFunction *function /* = nullptr */)
{
    m_clockUi->m_currentMenu = menu;
    m_clockUi->m_forceRefresh = true;
    m_clockUi->initHorizScrolling();

    m_clockUi->m_curFuncIdx = 0;

    // Skip unavailable functions
    while (!menu->at(m_clockUi->m_curFuncIdx)->isAvailable()) m_clockUi->m_curFuncIdx++;

    if (function != nullptr)
    {
        // Select the given function in the menu if it is found
        for (int i = 0; i < menu->size(); i++)
        {
            if (menu->at(i).get() == function)
            {
                m_clockUi->m_curFuncIdx = i;
                break;
            }
        }
    }
}

void AbstractFunction::select()
{
    for (int i = 0; i < m_clockUi->m_currentMenu->size(); i++)
    {
        if (m_clockUi->m_currentMenu->at(i).get() == this)
        {
            TRACE << "Found function";
            m_clockUi->m_curFuncIdx = i;
            break;
        }
    }
    m_clockUi->m_forceRefresh = true;
}

Buzzer &AbstractFunction::buzzer()
{
    return m_clockUi->m_buzzer;
}
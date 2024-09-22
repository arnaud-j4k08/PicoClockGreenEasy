#include "AbstractFunction.h"
#include "Utils/Trace.h"
#include "Clock.h"
#include "ClockUi.h"

#include <ctime>

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

void AbstractFunction::adjustField(
    Direction dir, FieldBehaviour behaviour, int &value, int daysInMonth)
{
    int bigStepSize = 0;
    int floor = 0;
    int modulo = 0;
    switch(behaviour)
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
            modulo = daysInMonth;
            floor = 1;
            bigStepSize = 7;
            break;
        case Hour:
            modulo = 24;
            bigStepSize = 6;
            break;
        case Minute:
        case Second:
            modulo = 60;
            bigStepSize = 10;
            break;
        case ManualBrightness:
            modulo = 101;
            bigStepSize = 10;
            break;
        case AutoBrightnessPoint:
            floor = -100;
            modulo = 201;
            bigStepSize = 10;
            break;
    }

    TRACE << "Previous value:" << value;
    value -= floor;
    switch (dir)
    {
        case Up:
            value = (value + 1) % modulo;
            break;
        case Down:
            value = (value + modulo - 1) % modulo;
            break;
        case RepeatedUp:
            // Round to the big step multiple or move up to the next one
            value = ((value + bigStepSize) % modulo) / bigStepSize * bigStepSize;
            break;
        case RepeatedDown:
            // Round to the big step multiple or move down to the next one
            value = ((value + modulo - 1) % modulo) / bigStepSize * bigStepSize;
            break;
    }
    value += floor;
    TRACE << "New value: " << value;
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

    menu->at(m_clockUi->m_curFuncIdx)->onSelect();
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
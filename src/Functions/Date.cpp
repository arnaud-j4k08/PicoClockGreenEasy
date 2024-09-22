#include "Date.h"
#include "Bitmap.h"
#include "Clock.h"

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

void Date::renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    if (fullRefresh || 
        (clock().tickCount() == 0 && clock().get().tm_hour == 0 && clock().get().tm_min == 0 && clock().get().tm_sec == 0) ||
        blinkingCounter == 0 ||
        blinkingCounter == BLINKING_DISAPPEAR_FRAME)
    {
        frame.clear();
        frame.setFont(&classicFont);

        if (editedValueIndex == EditingYear)
        {
            if (blinkingCounter < BLINKING_DISAPPEAR_FRAME)
                frame.drawText(1, 0, std::to_string(clock().get().tm_year + 1900));
        } 
        else
        {
            if (editedValueIndex != EditingDay || blinkingCounter<BLINKING_DISAPPEAR_FRAME)
            {
                frame.draw2DigitsIntWithLeadingZero(0, 0, clock().get().tm_mday);
            }

            frame.drawRectangle(10, 3, 11, 3, true);

            if (editedValueIndex != EditingMonth || blinkingCounter<BLINKING_DISAPPEAR_FRAME)
            {
                frame.draw2DigitsIntWithLeadingZero(13, 0, clock().get().tm_mon + 1);
            }
        }

        putWeekDay(frame);
    }
}

void Date::startEditingValue(int valueIndex)
{
    if (valueIndex == EditingYear || valueIndex == EditingMonth)
    {
        // Begin blinking with visible digits as the view just changed between year and
        // day-month
        setBlinkingCounter(0);
    }
}

void Date::modifyValue(int valueIndex, Direction direction)
{
    tm tm = clock().get();

    switch (valueIndex)
    {
        case EditingYear:
            adjustField(direction, Year, tm.tm_year);
            break;

        case EditingMonth:
            adjustField(direction, Month, tm.tm_mon);
            break;

        case EditingDay:
            adjustField(direction, Day, tm.tm_mday, daysInMonth(tm));
            break;
    }

    // Save time in the program clock. RTC sync will be triggered when leaving edit mode.
    clock().set(tm);
}

void Date::finishEditing()
{
    // Trigger sync to RTC at the next second change.
    clock().startSyncToRtc();
}

#include "Date.h"
#include "Bitmap.h"
#include "Clock.h"

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
    switch (valueIndex)
    {
        case EditingYear:
            adjustField(Year, direction);
            break;

        case EditingMonth:
            adjustField(Month, direction);
            break;

        case EditingDay:
            adjustField(Day, direction);
            break;
    }
}

void Date::finishEditing()
{
    // Trigger sync to RTC at the next second change.
    clock().startSyncToRtc();
}

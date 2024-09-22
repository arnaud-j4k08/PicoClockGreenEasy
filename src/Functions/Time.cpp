#include "Time.h"
#include "fonts.h"
#include "Bitmap.h"
#include "Clock.h"

void Time::renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    switch(m_style)
    {
    case HourMinSec:
        renderHourMinSec(frame, editedValueIndex, blinkingCounter, fullRefresh);
        break;
    case HourMinBar:
        renderHourMinProgressBar(frame, editedValueIndex, blinkingCounter, fullRefresh);
        break;
    case HourMin:
        renderHourMin(frame, editedValueIndex, blinkingCounter, fullRefresh);
        break;
    }
}

void Time::startEditingValue(int valueIndex)
{
    if (valueIndex == EditingHour)
    {
        // Begin blinking with hidden digits for immediate user feedback.
        setBlinkingCounter(BLINKING_DISAPPEAR_FRAME);
    }
}

void Time::finishEditing()
{
    // Trigger sync to RTC at the next second change.
    clock().startSyncToRtc();
}

void Time::renderHourMinSec(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    if (clock().tickCount() == 0 || 
        (editedValueIndex != NoEditing && blinkingCounter == 0) ||
        fullRefresh)
    {
        frame.clear();

        int displayedHour = putAmPmAndConvertCurrentHour(frame);
        if (displayedHour >= 10)
        {
            // Write the very first digit of the hour using the ultra narrow font as otherwise, 
            // there are not enough pixels for everthing to fit
            frame.setFont(&ultraNarrowFont);
            frame.drawChar(0, 0, '0' + displayedHour / 10);
        }

        frame.setFont(&narrowFont);
        frame.drawChar(3, 0, '0' + displayedHour % 10);

        frame.draw2DigitsIntWithLeadingZero(7, 0, clock().get().tm_min);
        frame.draw2DigitsIntWithLeadingZero(15, 0, clock().get().tm_sec);

        putWeekDay(frame);
    }

    // Hide blinking digits at the half of the blinking cycle, or if it was just displayed
    // above and it is not the time to be visible
    if (blinkingCounter == BLINKING_DISAPPEAR_FRAME || 
        (clock().tickCount() == 0 && blinkingCounter >= BLINKING_DISAPPEAR_FRAME))
    {
        switch (editedValueIndex)
        {
            case EditingHour:
                frame.drawRectangle(0, 0, 6, 7, false);
                break;
            case EditingMinute:
                frame.drawRectangle(7, 0, 13, 7, false);
                break;
        }
    } 

    // Blinking of double dots. Also redraw at the beginning of blinking cycle, as the full buffer was 
    // cleared above.
    if (clock().tickCount() == Display::FRAME_RATE / 2 || 
        (editedValueIndex != NoEditing && blinkingCounter == 0 && clock().tickCount() >= Display::FRAME_RATE / 2) || 
        fullRefresh)
    {
        frame.putPixel(6, 2, true);
        frame.putPixel(6, 4, true);
        frame.putPixel(14, 2, true);
        frame.putPixel(14, 4, true);
    }
}

void Time::renderHourMinProgressBar(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    int barWidth = clock().get().tm_sec / 3;

    if ((clock().get().tm_sec == 0 && clock().tickCount() == 0) || 
        (editedValueIndex != NoEditing && blinkingCounter == 0) || 
        fullRefresh)
    {
        frame.clear();
        fullRefresh = true;

        frame.setFont(&shortFont);

        frame.draw2DigitsInt(0, 0, putAmPmAndConvertCurrentHour(frame));
        frame.draw2DigitsIntWithLeadingZero(12, 0, clock().get().tm_min);

        frame.putPixel(10, 1, true);
        frame.putPixel(10, 3, true);

        frame.drawRectangle(0, 6, barWidth - 1, 6, true);
    }

   if (blinkingCounter == BLINKING_DISAPPEAR_FRAME)
    {
        switch (editedValueIndex)
        {
            case EditingHour:
                frame.drawRectangle(0, 0, 8, 4, false);
                break;
            case EditingMinute:
                frame.drawRectangle(12, 0, 20, 4, false);
                break;
        }
    }

    if (clock().tickCount() == 0 || fullRefresh)
    {
        switch(clock().get().tm_sec % 3)
        {
            case 0:
                if (barWidth != 0)
                    frame.putPixel(barWidth - 1, 6, true);

                // Remove pixel of the previous second
                frame.putPixel(barWidth, 6, false);
                break;
            case 1:
                frame.putPixel(barWidth + 2, 6, true);
                break;
            case 2:
                frame.putPixel(barWidth + 1, 6, true);

                // Remove pixel of the previous second
                frame.putPixel(barWidth + 2, 6, false);
                break;
        }
        putWeekDay(frame);
    }
}

void Time::renderHourMin(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    if (editedValueIndex != NoEditing && 
        (blinkingCounter == 0 || blinkingCounter == BLINKING_DISAPPEAR_FRAME))
        fullRefresh = true;

    if (fullRefresh || (clock().get().tm_sec == 0 && clock().tickCount() == 0))
    {
        frame.clear();
        frame.setFont(&classicFont);

        int displayedHour = putAmPmAndConvertCurrentHour(frame);
        if (editedValueIndex != EditingHour || blinkingCounter < BLINKING_DISAPPEAR_FRAME)
            frame.draw2DigitsInt(0, 0, displayedHour);

        if (editedValueIndex != EditingMinute || blinkingCounter < BLINKING_DISAPPEAR_FRAME)
            frame.draw2DigitsIntWithLeadingZero(13, 0, clock().get().tm_min);

    }

    if (fullRefresh || clock().tickCount() == 0)
    {
        bool dotVisible = clock().get().tm_sec % 2 != 0;
        frame.drawRectangle(10, 1, 11, 2, dotVisible);
        frame.drawRectangle(10, 4, 11, 5, dotVisible);

        putWeekDay(frame);
    }
}

int Time::putAmPmAndConvertCurrentHour(Bitmap &frame)
{
    int displayedHour;
    bool morning;
    convertHour(clock().get().tm_hour, displayedHour, morning);
    putAmPmIndicators(frame, morning);

    return displayedHour;
}

void Time::modifyValue(int valueIndex, Direction direction)
{
    tm tm = clock().get();

    switch(valueIndex)
    {
        case EditingHour:
            adjustField(direction, Hour, tm.tm_hour);
            break;

        case EditingMinute:
            adjustField(direction, Minute, tm.tm_min);

            // Changing minutes also resets seconds and ticks to 0. This way, the user can 
            // synchronize precisely with another clock.
            tm.tm_sec = 0;
            clock().resetTicks();
            break;
    }

    // Save time in the program clock. RTC sync will be triggered when leaving edit mode.
    clock().set(tm);
}

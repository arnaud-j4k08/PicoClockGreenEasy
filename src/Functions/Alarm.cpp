#include "Alarm.h"

#include "UiTexts.h"
#include "fonts.h"
#include "Bitmap.h"
#include "Clock.h"

namespace
{
    std::string alarmModeText(const Settings::Alarm &alarm)
    {
        TextId textId;
        switch(alarm.mode)
        {
            case Settings::AlarmMode::Off:   
                textId = TextId::Off;
                break;
            case Settings::AlarmMode::Loud:   
                textId = TextId::Loud;
                break;
            case Settings::AlarmMode::Gradual:
                textId = TextId::Gradual;
                break;
            default:
                return "";
        }
        return uiText(textId);
    }
}

void Alarm::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    std::string prefix = 
        m_alarmId == Alarm1 ? uiText(TextId::AlarmShortened1Colon) : uiText(TextId::AlarmShortened2Colon);
    auto &alarm = 
        m_alarmId == Alarm1 ? settings().alarm1 : settings().alarm2;

    int displayedHour;
    bool morning;
    convertHour(alarm.hour, displayedHour, morning);
    std::string hourS = std::to_string(displayedHour);
    
    // Add a leading space to hours, but not in view mode for better readability
    if (hourS.size() == 1 && editedValueIndex != NoEditing)
    {
        // Use a fix width space to avoid shifting of characters when changing numbers.
        hourS = std::string(1, FixedWidthSpace) + hourS;
    }

    std::string minS = std::to_string(alarm.min);
    if (minS.size() == 1)
        minS = '0' + minS;

    switch (editedValueIndex)
    {
        case NoEditing:
            if (alarm.mode == Settings::AlarmMode::Off)
                renderScrollingText(frame, fullRefresh, prefix + uiText(TextId::Off));
            else
            {
                renderScrollingText(frame, fullRefresh, prefix + hourS + ":" + minS);
                putAmPmIndicators(frame, morning);
            }
            break;
        case EditingAlarmMode:
            renderScrollingText(frame, fullRefresh, prefix, alarmModeText(alarm));
            break;
        case EditingAlarmHour:
            renderScrollingText(frame, fullRefresh, prefix, hourS, ":" + minS);
            putAmPmIndicators(frame, morning);
            break;
        case EditingAlarmMinute:
            renderScrollingText(frame, fullRefresh, prefix + hourS + ":", minS);
            putAmPmIndicators(frame, morning);
            break;
        case EditingAlarmWeekDays:
            // Stick to the right to avoid the disturbing scrolling while the user is editing days.
            bringScrollingToRight();
            
            renderScrollingText(frame, fullRefresh, prefix + hourS + ":" + minS);
            putAmPmIndicators(frame, morning);
            break;
    }
    
    if (alarm.mode != Settings::AlarmMode::Off)
        frame.putWeekDays(alarm.weekDayBits);
    
    if (editedValueIndex == EditingAlarmWeekDays)
    {
        bool on = blinkingCounter < BLINKING_DISAPPEAR_FRAME / 2;

        if (alarm.enabledOnWeekDay(m_editedAlarmWeekDay))
            on = !on;

        frame.putWeekDay(m_editedAlarmWeekDay, on);
    }
}

int Alarm::valueCount() const
{
    if (alarmSettings().mode == Settings::AlarmMode::Off)
    {
        // As the alarm is off, only the mode can be edited. Time/weekdays values are not accessible.
        return 2;
    } else
    {
        return ValueCount;
    }
}

void Alarm::startEditingValue(int valueIndex)
{
    switch (valueIndex)
    {
        case EditingAlarmMode:
        case EditingAlarmHour:
        case EditingAlarmMinute:
            bringScrollingToRight();
            break;
        case EditingAlarmWeekDays:
            m_editedAlarmWeekDay = 1; // Monday
            break;
    }
}

const Settings::Alarm &Alarm::alarmSettings() const
{
    if (m_alarmId == Alarm1)
        return settings().alarm1;
    else
        return settings().alarm2;
}

Settings::Alarm &Alarm::modifyAlarmSettings()
{
    if (m_alarmId == Alarm1)
        return modifySettings().alarm1;
    else
        return modifySettings().alarm2;
}

void Alarm::modifyValue(int valueIndex, Direction direction)
{
    switch(valueIndex)
    {
    case EditingAlarmMode:
        adjustEnum(modifyAlarmSettings().mode, direction);
        break;

    case EditingAlarmHour:
        adjustField(m_alarmId == Alarm1 ? Alarm1Hour : Alarm2Hour, direction);
        break;

    case EditingAlarmMinute:
        adjustField(m_alarmId == Alarm1 ? Alarm1Minute : Alarm2Minute, direction);
        break;
    
    case EditingAlarmWeekDays:
        if (direction == Up || direction == RepeatedUp)
        {
            // Middle button cycles through weekdays
            m_editedAlarmWeekDay.increment();
        } else
        {
            // Bottom button toggle the selected weekday
            modifyAlarmSettings().weekDayBits ^= 1 << m_editedAlarmWeekDay;
        }
        break;
    }
}

void Alarm::finishEditing()
{
    auto &alarm = modifyAlarmSettings();

    // If no week days were selected, disable the alarm to avoid confusion.
    if (alarm.weekDayBits == 0)
        alarm.mode = Settings::AlarmMode::Off;

    // Set the alarm in the clock so that it can ring.
    if (m_alarmId == Alarm1)
        clock().setAlarm(Clock::Alarm1, alarm);
    else
        clock().setAlarm(Clock::Alarm2, alarm);
}

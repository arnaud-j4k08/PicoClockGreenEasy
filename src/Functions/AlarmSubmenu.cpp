#include "AlarmSubmenu.h"
#include "UiTexts.h"
#include "Clock.h"

AlarmSubmenu::AlarmSubmenu(
    ClockUi *clockUi, 
    std::vector<std::unique_ptr<AbstractFunction>> *parentMenu) 
    : Submenu(clockUi, uiText(TextId::Alarms), parentMenu)
{}

void AlarmSubmenu::onSelect()
{
    std::string text = uiText(TextId::Alarms);
 
    int hour = -1, min = -1;
    if (clock().nextAlarm(m_nextAlarmWeekday, hour, min, settings()))
    {
        int displayedHour;
        convertHour(hour, displayedHour, m_nextAlarmOnMorning);

        std::string minS = std::to_string(min);
        if (minS.size() == 1)
            minS = '0' + minS;

        text += 
            " (" + 
            uiText(TextId::NextColon) + 
            std::to_string(displayedHour) + 
            ":" + 
            minS +
            ")";
    }

    setName(text);
}

void AlarmSubmenu::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    Submenu::renderFrame(frame, editedValueIndex, blinkingCounter, fullRefresh);

    if (clock().isAlarmOn())
    {
        putAmPmIndicators(frame, m_nextAlarmOnMorning);
        frame.putWeekDay(m_nextAlarmWeekday, true);
    }
}

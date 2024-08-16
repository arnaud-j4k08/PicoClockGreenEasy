#include "SkipNextAlarm.h"
#include "UiTexts.h"
#include "Clock.h"

bool SkipNextAlarm::isAvailable() const
{
    return clock().isAlarmOn();
}

void SkipNextAlarm::renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    std::string state = settings().skipNextAlarm ? uiText(TextId::On) : uiText(TextId::Off);
    renderScrollingText(frame, fullRefresh, uiText(TextId::SkipNextAlarmColon) + state);
}

void SkipNextAlarm::activate()
{
    modifySettings().skipNextAlarm = !settings().skipNextAlarm;
    bringScrollingToRight();
}

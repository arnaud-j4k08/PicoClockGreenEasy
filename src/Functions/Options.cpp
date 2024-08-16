#include "Options.h"
#include "UiTexts.h"
#include "Utils/Trace.h"

namespace
{
    void toggleBool(bool &option)
    {
        option = !option;
    }
}

void Options::renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    switch(editedValueIndex)
    {
        case NoEditing:
            renderScrollingText(frame, fullRefresh, uiText(TextId::Options));
            break;
        case EditingAutoScroll:
            renderScrollingText(
                frame, 
                fullRefresh, 
                uiText(TextId::AutoScrollColon), 
                settings().autoScroll ? uiText(TextId::On) : uiText(TextId::Off));
            break;
        case EditingFormat:
            renderScrollingText(
                frame, 
                fullRefresh, 
                uiText(TextId::FormatColon), 
                settings().format24h ? uiText(TextId::Format24h) : uiText(TextId::Format12h));
            break;
        case EditingHourlyChime:
            TextId modeTextId;
            switch(settings().hourlyChime)
            {
                case Settings::HourlyChimeMode::Off:
                    modeTextId = TextId::Off;
                    break;
                case Settings::HourlyChimeMode::On:
                    modeTextId = TextId::On;
                    break;
                case Settings::HourlyChimeMode::OnDayLight:
                    modeTextId = TextId::Day;
                    break;
            }
            renderScrollingText(
                frame, fullRefresh, uiText(TextId::HourlyChimeColon), uiText(modeTextId));
            break;
        case EditingAutoLight:
            renderScrollingText(
                frame, 
                fullRefresh, 
                uiText(TextId::AutoLightColon), 
                settings().autoLight ? uiText(TextId::On) : uiText(TextId::Off));
            break;
    }
}

void Options::modifyValue(int valueIndex, Direction direction)
{
    switch(valueIndex)
    {
        case EditingAutoScroll:
            toggleBool(modifySettings().autoScroll);
            break;

        case EditingFormat:
            toggleBool(modifySettings().format24h);
            break;

        case EditingHourlyChime:
            adjustEnum(modifySettings().hourlyChime, direction);
            break;

        case EditingAutoLight:
            toggleBool(modifySettings().autoLight);
            break;
    }
}

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
        case EditingManualBrightness:
            if (settings().autoLight)
            {
                renderScrollingText(
                    frame, 
                    fullRefresh, 
                    uiText(TextId::BrightnessDarkColon), 
                    std::to_string(settings().brightnessDark) + "%");
            } else
            {
                renderScrollingText(
                    frame, 
                    fullRefresh, 
                    uiText(TextId::BrightnessColon), 
                    std::to_string(settings().manualBrightness) + "%");
            }
            break;
        case EditingBrightnessDim:
            renderScrollingText(
                frame, 
                fullRefresh, 
                uiText(TextId::BrightnessDimColon), 
                std::to_string(settings().brightnessDim) + "%");
            break;
        case EditingBrightnessBright:
            renderScrollingText(
                frame, 
                fullRefresh, 
                uiText(TextId::BrightnessBrightColon), 
                std::to_string(settings().brightnessBright) + "%");
            break;
    }
}

int Options::valueCount() const 
{
    if (settings().autoLight)
        return ValueCount;
    else
        return EditingManualBrightness + 1; // No more editable value after this one
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

        case EditingManualBrightness:
            if (settings().autoLight)
                adjustField(BrightnessDark, direction);
            else
                adjustField(ManualBrightness, direction);
            break;

        case EditingBrightnessDim:
            adjustField(BrightnessDim, direction);
            break;

        case EditingBrightnessBright:
            adjustField(BrightnessBright, direction);
            break;
    }
}

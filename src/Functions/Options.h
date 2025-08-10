#pragma once

#include "AbstractFunction.h"

class Bitmap;

class Options : public AbstractFunction
{
public:
    Options(ClockUi *clockUi) : AbstractFunction(clockUi)
    {}

private:
    enum EditedValue
    {
        NoEditing = 0,
        EditingAutoScroll,
        EditingDigitAnimation,
        EditingTimeFormat,
        EditingDateFormat,
        EditingHourlyChimeMode,
        EditingChimeVolume, // Only if hourly chime mode is set to "Sound" or "SoundOnDayLight"
        EditingAutoLight,

        // Value to edit if auto light is disabled
        EditingManualBrightness,
        
        // Values to edit if auto light is enabled
        EditingBrightnessDark, 
        EditingBrightnessDim,
        EditingBrightnessBright,

        ValueCount
    };

    void renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    int valueCount() const override
    {
        return ValueCount;
    }
    bool isValueAvailable(int valueIndex) const override;
    void modifyValue(int valueIndex, Direction direction) override;

    BrightnessHandling brightnessHandling(int valueIndex) const override
    {
        // Disable "brightness boost" when setting brightness so that the user can see the actual 
        // brightness that results from the setting.
        if (valueIndex != EditingBrightnessDark && 
            valueIndex != EditingBrightnessDim && 
            valueIndex != EditingBrightnessBright)
        {
            return WithBoost;
        } else
        {
            return WithoutBoost;
        }
    }
};
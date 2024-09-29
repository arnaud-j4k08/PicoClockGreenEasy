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
        EditingFormat,
        EditingHourlyChime,
        EditingAutoLight,

        // Value to edit if auto light is disabled
        EditingManualBrightness,
        
        // Values to edit if auto light is enabled
        EditingBrightnessDark = EditingManualBrightness, 
        EditingBrightnessDim,
        EditingBrightnessBright,

        ValueCount
    };

    void renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    int valueCount() const override;
    void modifyValue(int valueIndex, Direction direction) override;

    bool allowsBrightnessBoost(int valueIndex) const override
    {
        // Disable "brightness boost" when setting brightness so that the user can see the actual 
        // brightness that results from the setting.
        return 
            valueIndex != EditingBrightnessDark && 
            valueIndex != EditingBrightnessDim && 
            valueIndex != EditingBrightnessBright;
    }
};
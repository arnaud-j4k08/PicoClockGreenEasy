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
        ValueCount
    };

    void renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    int valueCount() const override
    {
        return ValueCount;
    }
    void modifyValue(int valueIndex, Direction direction) override;
};
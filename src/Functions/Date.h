#pragma once

#include "AbstractFunction.h"

class Bitmap;

class Date : public AbstractFunction
{
public:
    Date(ClockUi *clockUi) : AbstractFunction(clockUi)
    {}

private:
    enum EditedValue
    {
        NoEditing = 0,
        EditingYear,
        EditingMonth,
        EditingDay,
        ValueCount
    };

    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    int valueCount() const override
    {
        return ValueCount;
    }
    void startEditingValue(int valueIndex) override;
    void modifyValue(int valueIndex, Direction direction) override;
    void finishEditing() override;
};
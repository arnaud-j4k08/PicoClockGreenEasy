#pragma once

#include "AbstractFunction.h"

class Bitmap;

class Time : public AbstractFunction
{
public:
    enum Style
    {
        HourMinSec,
        HourMinBar, // Hour, minutes and a bar acting as horizontal hourglass
        HourMin
    };

    Time(ClockUi *clockUi, Style style) : AbstractFunction(clockUi), m_style(style)
    {}

private:
    enum EditedValue
    {
        NoEditing = 0,
        EditingHour,
        EditingMinute,
        ValueCount
    };

    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    bool isTimeFunction() const override
    {
        return true;
    }
    int valueCount() const override
    {
        return ValueCount;
    }
    void startEditingValue(int valueIndex) override;
    void modifyValue(int valueIndex, Direction direction) override;
    void finishEditing() override;

    void renderHourMinSec(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh);
    void renderHourMinProgressBar(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh);
    void renderHourMin(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh);
    int putAmPmAndConvertCurrentHour(Bitmap &frame);

    const Style m_style;
};
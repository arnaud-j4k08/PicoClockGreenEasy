#pragma once

#include "AbstractFunction.h"

class SkipNextAlarm : public AbstractFunction
{
public:
    SkipNextAlarm(ClockUi *clockUi) : AbstractFunction(clockUi)
    {}

private:
    void renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;

    bool isAvailable() const override;
    void activate() override;
};
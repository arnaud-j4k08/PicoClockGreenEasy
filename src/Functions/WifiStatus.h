#pragma once

#include "AbstractFunction.h"

class Bitmap;

class WifiStatus : public AbstractFunction
{
public:
    WifiStatus(ClockUi *clockUi) : AbstractFunction(clockUi)
    {}

private:
    bool isAvailable() const override;
    void renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    int valueCount() const override
    {
        return 1; // Nothing to edit, only the wifi status is displayed
    }
};
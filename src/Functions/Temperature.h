#pragma once

#include "AbstractFunction.h"

class Temperature : public AbstractFunction
{
public:
    Temperature(ClockUi *clockUi) : AbstractFunction(clockUi)
    {}

private:
    bool isAvailable() const override;
    void renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    void activate() override;
    int valueCount() const override
    {
        return 1; // Nothing to edit, only the temperature
    }    
};
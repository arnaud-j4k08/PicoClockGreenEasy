#pragma once

#include "AbstractFunction.h"
#include "AnimatedChar.h"
#include <math.h>

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

    float m_lastMeasuredTempCelsius = NAN; // Temperature not measured yet
    float m_displayedTemp = NAN;
    AnimatedChar m_digit1 { 2, 0};
    AnimatedChar m_digit2 { 7, 0};
    AnimatedChar m_digit3 { 14, 0};
};
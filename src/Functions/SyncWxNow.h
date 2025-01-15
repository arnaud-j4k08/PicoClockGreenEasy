#pragma once

#include "AbstractFunction.h"
#include <string>

class SyncWxNow : public AbstractFunction
{
public:
    SyncWxNow(ClockUi *clockUi) : AbstractFunction(clockUi)
    {}

    void setNextFunction(AbstractFunction *nextFunction)
    {
        m_nextFunction = nextFunction;
    }

private:
    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;

    void activate() override;

    AbstractFunction *m_nextFunction = nullptr;
};
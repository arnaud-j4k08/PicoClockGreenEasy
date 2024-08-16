#pragma once

#include "AbstractFunction.h"
#include "Utils/CyclicCounter.h"

class Stopwatch : public AbstractFunction
{
public:
    Stopwatch(ClockUi *clockUi) : AbstractFunction(clockUi)
    {}
    void tick();
    void reset();

private:
    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;

    void activate() override
    {
        m_running = !m_running;
    }

    bool m_running = false;
    CyclicCounter m_min {60};
    CyclicCounter m_sec {60};
    CyclicCounter m_tick {Display::FRAME_RATE};
};

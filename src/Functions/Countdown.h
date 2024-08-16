#pragma once

#include "AbstractFunction.h"
#include "Utils/CyclicCounter.h"

class Countdown : public AbstractFunction
{
public:
    Countdown(ClockUi *clockUi, std::vector<std::unique_ptr<AbstractFunction>> *parentMenu);
    void tick();
    void set();
    bool stopRinging(); // Return true if ringing has been stopped

private:
    enum EditedValue
    {
        NoEditing = 0,
        EditingMin,
        EditingSec,
        ValueCount
    };

    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;

    int valueCount() const override
    {
        return ValueCount;
    }

    void activate() override;
    void modifyValue(int valueIndex, Direction direction) override;

    void initCount();

    std::vector<std::unique_ptr<AbstractFunction>> *m_parentMenu;
    
    enum State
    {
        Stopped,
        Running,
        Ringing
    } m_state = Stopped;
    int m_min;
    CyclicCounter m_sec {60};
    CyclicCounter m_tick {Display::FRAME_RATE};
    CyclicCounter m_ringingCounter {Display::FRAME_RATE}; 
    int m_ringingForSecs = 0;
};
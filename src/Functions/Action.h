#pragma once

#include "AbstractFunction.h"
#include <string>

class Action : public AbstractFunction
{
public:
    Action(ClockUi *clockUi, const std::string &name, const std::function<void()> callback) :
        AbstractFunction(clockUi), m_name(name), m_callback(callback)
    {}

private:
    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override
    {
        renderScrollingText(frame, fullRefresh, m_name);
    }

    void activate() override
    {
        m_callback();
    }

    std::string m_name;
    std::function<void()> m_callback;
};
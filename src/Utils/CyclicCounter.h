#pragma once

class CyclicCounter
{
public:
    CyclicCounter(int wrap, int initialValue = 0) : m_wrap(wrap), m_value(initialValue)
    {}

    int wrapValue() const
    {
        return m_wrap;
    }

    // Return true if the counter has wrapped
    bool increment()
    {
        m_value++;
        if (m_value >= m_wrap)
        {
            m_value = 0;
            return true;
        } else
            return false;
    }

    // Return true if the counter has wrapped
    bool decrement()
    {
        m_value--;
        if (m_value < 0)
        {
            m_value = m_wrap - 1;
            return true;
        } else
            return false;
    }

    operator int() const
    {
        return m_value;
    }

    void operator = (int value)
    {
        m_value = value;
    }
    
private:
    const int m_wrap;
    int m_value = 0;
};
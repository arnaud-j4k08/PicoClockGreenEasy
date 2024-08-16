#pragma once

 #include <cstddef>
 #include "CyclicCounter.h"

template <size_t size>
class MovingAverage
{
public:
    MovingAverage(float initValue)
    {
        for (int i = 0; i < size; i++)
            m_ringBuffer[i] = initValue;

        m_sum = initValue * size;
    }

    void put(float value)
    {
        // Update the sum and exchange values in the ring buffer
        m_sum -= m_ringBuffer[m_pos];
        m_sum += value;
        m_ringBuffer[m_pos] = value;
        m_pos.increment();
    }

    float get() const
    {
        return m_sum / size;
    }

private:
    float m_ringBuffer[size];
    CyclicCounter m_pos{size, 0};
    float m_sum;
};
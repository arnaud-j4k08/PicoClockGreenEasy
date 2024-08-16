#pragma once

#include "Utils/MovingAverage.h"

#include <memory>

class Rtc
{
public:
    Rtc();
    ~Rtc();

    static void onSecond();

    bool read(tm &dateTime) const;
    bool write(const tm &dateTime);

    float temperature();

private:
    float rawTemperature();

    static Rtc *m_instance;
    bool m_communicating = false;
    bool m_requestTemp = true; // Request one temperature measurement at the beginning

    std::unique_ptr<MovingAverage<32>> m_tempFilter;
};


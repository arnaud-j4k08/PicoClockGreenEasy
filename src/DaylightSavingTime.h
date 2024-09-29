#pragma once

#include <time.h>

class DaylightSavingTime
{
public:
    enum Location
    {
        Unknown,
        Europe
    };

    time_t considerDst(time_t time);
    time_t unconsiderDst(time_t time);

private:
    bool isDstActive(time_t time);

    time_t m_yearStart = 0;
    time_t m_dstStart = 0;
    time_t m_dstEnd = 0;
    bool m_wasDstActive = false;
};
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

    void considerDst(time_t time, time_t &timeConsideringDst, bool &dstActive);
    void unapplyDst(time_t &time);

private:
    bool isDstActive(time_t time);

    time_t m_yearStart = 0;
    time_t m_dstStart = 0;
    time_t m_dstEnd = 0;
};
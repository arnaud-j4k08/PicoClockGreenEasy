#include "DaylightSavingTime.h"
#include "Clock.h"
#include "Utils/Trace.h"

time_t DaylightSavingTime::considerDst(time_t time)
{
    // Remember if DST is active, as we may need this information in unapplyDst
    m_wasDstActive = isDstActive(time);
    if (m_wasDstActive)
    {
        TRACE << "DST applied";
        return time + 3600;
    } else
        return time;
}

time_t DaylightSavingTime::unconsiderDst(time_t time)
{
    // Since DST may already have been applied to this time, check if it would be active with and
    // without DST correction.
    bool dstActive = isDstActive(time);
    bool dstActiveAssumingApplied = isDstActive(time - 3600);

    if (dstActive != dstActiveAssumingApplied)
    {
        TRACE << "In DST transition, use previous DST active flag";
        dstActive = m_wasDstActive;
    }

    if (dstActive)
        return time - 3600;
    else
        return time;
}

bool DaylightSavingTime::isDstActive(time_t time)
{
    if (DST_LOCATION != Europe)
        return false;

    // Determine DST start and end for the year if not done yet, or if the year has changed since we
    // last determined it.
    if (m_yearStart == 0 || 
        time < m_yearStart || 
        time >= m_yearStart + 366 * 24 * 3600) // Definitely in the next year, even if it was a leap year
    {
        tm givenTm = *localtime(&time);

        tm yearStartTm = givenTm;
        yearStartTm.tm_sec =0 ;
        yearStartTm.tm_min = 0;
        yearStartTm.tm_hour = 0;
        yearStartTm.tm_mday = 1;
        yearStartTm.tm_mon = 0;
        m_yearStart = mktime(&yearStartTm);

        // In Europe, DST starts on last Sunday in March at 01:00 UTC
        tm dstStartTm = givenTm;
        dstStartTm.tm_sec = 0;
        dstStartTm.tm_min = UTC_OFFSET * 60 % 60;
        dstStartTm.tm_hour = 1 + UTC_OFFSET;
        dstStartTm.tm_mon = 2; // March as "months since January"
        dstStartTm.tm_mday = 31; // Last day of March
        mktime(&dstStartTm); // So that dstStartTm.tm_wday is calculated
        dstStartTm.tm_mday -= dstStartTm.tm_wday; // last Sunday in March
        m_dstStart = mktime(&dstStartTm);
        TRACE << "This year, DST starts on" << dstStartTm;

        // In Europe, DST ends on last Sunday in October at 01:00 UTC
        tm dstEndTm = givenTm;
        dstEndTm.tm_sec = 0;
        dstEndTm.tm_min = UTC_OFFSET * 60 % 60;
        dstEndTm.tm_hour = 1 + UTC_OFFSET;
        dstEndTm.tm_mon = 9; // October as "months since January"
        dstEndTm.tm_mday = 31; // Last day of October
        mktime(&dstEndTm); // So that dstEndTm.tm_wday is calculated
        dstEndTm.tm_mday -= dstEndTm.tm_wday; // last Sunday in October
        m_dstEnd = mktime(&dstEndTm);
        TRACE << "This year, DST ends on" << dstEndTm;
    }

    bool dstActive = time >= m_dstStart && time < m_dstEnd;
    return dstActive;
}
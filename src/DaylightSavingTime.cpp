#include "DaylightSavingTime.h"
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
    if (DST_LOCATION == Unknown)
        return false;

    // Determine DST start and end for the year if not done yet, or if the year has changed since we
    // last determined it.
    if (m_yearStart == 0 || 
        time < m_yearStart || 
        time >= m_yearStart + 366 * 24 * 3600) // Definitely in the next year, even if it was a leap year
    {
        tm givenTm = *localtime(&time);
		
        // Disable DST consideration of mktime (for unit tests, it makes no difference with the 
        // Pico SDK)
        givenTm.tm_isdst = 0; 

        tm yearStartTm = givenTm;
        yearStartTm.tm_sec =0 ;
        yearStartTm.tm_min = 0;
        yearStartTm.tm_hour = 0;
        yearStartTm.tm_mday = 1;
        yearStartTm.tm_mon = 0;
        m_yearStart = mktime(&yearStartTm);

        determineDstStartAndEnd(givenTm);
    }

    return time >= m_dstStart && time < m_dstEnd;
}

void DaylightSavingTime::determineDstStartAndEnd(const tm &givenTm)
{
    // Reference: https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
    tm dstStartTm = givenTm;
    tm dstEndTm = givenTm;
    if (DST_LOCATION == Europe)
    {

        int utcOffsetMinutes = static_cast<int>(m_utcOffset * 60);

        // In Europe, DST starts on last Sunday in March at 01:00 UTC
        dstStartTm.tm_sec = 0;
        dstStartTm.tm_min = utcOffsetMinutes % 60;
        dstStartTm.tm_hour = 1 + static_cast<int>(m_utcOffset);
        dstStartTm.tm_mon = 2; // March as "months since January"
        dstStartTm.tm_mday = 31; // Last day of March
        mktime(&dstStartTm); // So that dstStartTm.tm_wday is calculated
        dstStartTm.tm_mday -= dstStartTm.tm_wday; // last Sunday in March

        // In Europe, DST ends on last Sunday in October at 01:00 UTC
        dstEndTm.tm_sec = 0;
        dstEndTm.tm_min = utcOffsetMinutes % 60;
        dstEndTm.tm_hour = 1 + static_cast<int>(m_utcOffset);
        dstEndTm.tm_mon = 9; // October as "months since January"
        dstEndTm.tm_mday = 31; // Last day of October
        mktime(&dstEndTm); // So that dstEndTm.tm_wday is calculated
        dstEndTm.tm_mday -= dstEndTm.tm_wday; // last Sunday in October
    } else // USA
    {
        // In USA, DST starts on second Sunday in March at 02:00 
        dstStartTm.tm_sec = 0;
        dstStartTm.tm_min = 0;
        dstStartTm.tm_hour = 2;
        dstStartTm.tm_mon = 2; // March as "months since January"
        dstStartTm.tm_mday = 1;
        mktime(&dstStartTm); // So that dstStartTm.tm_wday is calculated
        if (dstStartTm.tm_wday == 0)
            dstStartTm.tm_mday = 8; // The month starts with a Sunday, this is the second one
        else
            dstStartTm.tm_mday = 15 - dstStartTm.tm_wday; // second Sunday

        // In USA, DST ends on first Sunday in November at 02:00
        dstEndTm.tm_sec = 0;
        dstEndTm.tm_min = 0;
        dstEndTm.tm_hour = 1; // 02:00 with DST applied
        dstEndTm.tm_mon = 10; // November as "months since January"
        dstEndTm.tm_mday = 1;
        mktime(&dstEndTm); // So that dstEndTm.tm_wday is calculated
        if (dstEndTm.tm_wday == 0)
            dstEndTm.tm_mday = 1; // The month starts with a Sunday
        else
            dstEndTm.tm_mday = 8 - dstEndTm.tm_wday; // first Sunday    
    }

	m_dstStart = mktime(&dstStartTm);
    TRACE << "This year, DST starts on" << dstStartTm;
	m_dstEnd = mktime(&dstEndTm);
    TRACE << "This year, DST ends on" << dstEndTm;
}
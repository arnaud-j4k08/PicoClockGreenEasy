#include "Clock.h"
#include "Utils/Trace.h"

Clock::Clock(int tickPerSec) : 
    m_tickCount(tickPerSec), m_rtc(std::make_unique<Rtc>()), m_ntp(std::make_unique<Ntp>())
{
    // Initialize m_time and m_tm from the RTC. It will be used for displaying time 
    // while waiting for the sync from the RTC to be finished.
    tm rtcTime;
    if (m_rtc->read(rtcTime))
    {
        set(rtcTime);
        m_rtcSync = SyncingFromRtc;

        TRACE << "Set m_lastRtcSec to be able to detect when the second changes in the RTC";
        m_lastRtcSec = rtcTime.tm_sec;
    } else
    {
        TRACE << "No RTC available";
        m_rtc.release();
        m_rtcSync = SyncDone;
    }
}

void Clock::startSyncFromNtp()
{
    if (!m_ntp)
        return;

    if (!m_ntp->init())
    {
        // Will not use NTP.
        m_ntp.release();
        return;
    }

    using namespace std::placeholders;
    m_ntp->setTimeCallback(std::bind(&Clock::onNtpTimeReceived, this, _1, _2));
    m_ntp->startRequest();
}

void Clock::onNtpTimeReceived(time_t utcTime, uint32_t ms)
{
    TRACE << "Received ntp time:" << utcTime <<", setting it";
    m_time = utcTime + UTC_OFFSET * 60 * 60;
    m_tm = *localtime(&m_time);
    m_tickCount = ms * m_tickCount.wrapValue() / 1000;
    m_clockAdjusted = true;

    // Now that we got the time from NTP, plan RTC sync at the next second change.
    m_rtcSync = SyncingToRtc;
}

void Clock::tick(bool &clockAdjusted, AlarmId &reachedAlarm)
{
    clockAdjusted = false;
    reachedAlarm = NoAlarm;

    m_tickCount.increment();

    if (m_rtc && m_rtcSync == SyncingFromRtc) // RTC available and synchronizing with it?
    {
        TRACE << "Synchronizing with RTC";
        tm rtcTime;
        if (m_rtc->read(rtcTime))
        {
            if (rtcTime.tm_sec != m_lastRtcSec)
            {
                TRACE << "done";
                // The second just changed in the RTC, synchronize.
                // Note: the mktime function used by the set method is quite slow and would cause a 
                // glitch in the display if it was called on every frame. Therefore it is only 
                // called at the end of the sync here.
                set(rtcTime);
                m_tickCount = 0;
                m_rtcSync = SyncDone;
            }
        } else
        {
            // RTC read failed, give up with synchronization
            m_rtcSync = SyncDone;
        }
    } else
    {
        // Count time in the program. No longer read from the RTC. Update it if needed.
        if (m_tickCount == 0)
        {
            m_time++;
            m_tm = *localtime(&m_time);

            if (m_rtc && m_rtcSync == SyncingToRtc)
            {
                TRACE << "Set RTC";
                if (m_rtc->write(m_tm))
                    m_rtcSync = SyncDone;
            }
        }
    }   

    if (m_tickCount == 0 && m_tm.tm_sec == 0)
    {
        if (alarmReached(Alarm1))
            reachedAlarm = Alarm1;
        else if (alarmReached(Alarm2))
            reachedAlarm = Alarm2;
    }

    if (m_clockAdjusted)
    {
        m_clockAdjusted = false;
        clockAdjusted = true;
    } 
}

void Clock::set(const tm &tm)
{
    m_tm = tm;
    m_time = mktime(&m_tm);
    m_clockAdjusted = true;
}

void Clock::setAlarm(AlarmId id, const Settings::Alarm &al)
{
    m_alarm[id] = al;
}

bool Clock::alarmReached(AlarmId id) const
{
    const Settings::Alarm &al = m_alarm[id];
    return 
        m_tm.tm_min == al.min && 
        m_tm.tm_hour == al.hour && 
        al.enabledOnWeekDay(m_tm.tm_wday) && 
        al.mode != Settings::AlarmMode::Off;
}

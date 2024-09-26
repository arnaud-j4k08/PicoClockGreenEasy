#pragma once

#include "DaylightSavingTime.h"
#include "PicoClockHw/Rtc.h"
#include "PicoClockHw/Ntp.h"
#include "Settings.h"
#include "Utils/CyclicCounter.h"

#include <memory>
#include <time.h>

class Clock
{
public:
    enum AlarmId
    {
        NoAlarm = -1,
        Alarm1 = 0,
        Alarm2 = 1,
        AlarmCount
    };

    Clock(int tickPerSec);

    void startSyncFromNtp();
    void startSyncToRtc()
    {
        m_rtcSync = SyncingToRtc;
    }
    
    void tick(bool &clockAdjusted, AlarmId &reachedAlarm, Settings &settings);
    void setAlarm(AlarmId id, const Settings::Alarm &al);
    bool nextAlarm(int &weekday, int &hour, int &min, const Settings::Values &settings) const;

    bool isAlarmOn() const
    {
        return 
            m_alarm[Alarm1].mode != Settings::AlarmMode::Off || 
            m_alarm[Alarm2].mode != Settings::AlarmMode::Off;
    }
    int tickCount() const
    {
        return m_tickCount;
    }
    void resetTicks()
    {
        m_tickCount = 0;
    }
    const tm &get() const
    {
        return m_tm;
    }
    void set(const tm &tm);
    
    bool hasRtc() const
    {
        return m_rtc.operator bool();
    }

    Rtc *rtc()
    {
        return m_rtc.get();
    }

private:
    struct Time
    {
        // Initialize with too high values, so that isValid returns false by default, and another
        // object with valid time is regarded as smaller.
        int hour = 99;
        int min = 99;

        bool operator <(const Time &other) const;
        bool operator <=(const Time &other) const;
        bool isValid() const;
    };

    void onNtpTimeReceived(time_t utcTime, uint32_t ms);
    bool alarmReached(AlarmId id) const;
    bool nextAlarmAfter(
        int startWeekday, const Time &startTime, int &weekday, Clock::Time &time) const;
    Time alarmTimeAtDay(AlarmId alarmId, int weekday) const;
    void setTmFromTime();
    void setFromNonDstConsideringTm(tm tm); 

    enum RtcSync
    {
        SyncingFromRtc,
        SyncingToRtc,
        SyncDone
    };

    CyclicCounter m_tickCount;
    std::unique_ptr<Rtc> m_rtc; // As unique_ptr so that it can be easily disabled
    std::unique_ptr<Ntp> m_ntp;
    RtcSync m_rtcSync = SyncingFromRtc;
    int m_lastRtcSec;
    Settings::Alarm m_alarm[AlarmCount];

    DaylightSavingTime m_dst;
    
    time_t m_time = 0; // Current time as unix time, local (not UTC), not considering DST
    tm m_tm = {}; // Current time as tm, considering DST
    bool m_dstActive = false;

    bool m_clockAdjusted = true;
};
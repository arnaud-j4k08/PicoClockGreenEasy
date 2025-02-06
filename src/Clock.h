#pragma once

#include "DaylightSavingTime.h"
#include "PicoClockHw/Gps.h"
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

    struct SyncInfo
    {
        int dailySyncHour = 0;
        int dailySyncMin = 0;
        tm lastSyncTm = {};
        Settings::SyncSource lastSyncSource = Settings::SyncSource::Rtc;
        int lastSyncDriftMs = 0;
    };


    // Beware that the object keeps a reference on settings, so it must exists at least as long as
    // the object.
    Clock(int tickPerSec, Settings &settings);

    void onWifiInited();
    void startSyncToRtc()
    {
        m_rtcSync = SyncingToRtc;
    }
    
    void tick(bool &clockAdjusted, Settings::AlarmMode &reachedAlarmMode); 
    bool nextAlarm(int &weekday, int &hour, int &min) const;

    bool isAlarmOn() const
    {
        return 
            alarm(Alarm1).mode != Settings::AlarmMode::Off || 
            alarm(Alarm2).mode != Settings::AlarmMode::Off;
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

    bool isSynchronizing() const
    {
        return m_rtcSync == SyncingFromRtc || m_extSync != Inactive;
    }

    void syncNow();
    void syncInfo(SyncInfo &info);

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

    void onExternalTimeReceived(time_t utcTime, uint32_t ms, Settings::SyncSource source);
    Settings::AlarmMode checkIfAlarmReached();
    bool alarmReached(AlarmId id) const;
    const Settings::Alarm &alarm(AlarmId id) const;
    bool nextAlarmAfter(
        int startWeekday, 
        const Time &startTime, 
        int &weekday, 
        Clock::Time &time, 
        unsigned int &enabledAlarmsMask) const;
    bool earliestAlarm(
        const Time &alarm1Time,
        const Time &alarm2Time,
        Time &earliestTime,
        unsigned int &enabledAlarmsMask) const;
    Time alarmTimeAtDay(AlarmId alarmId, int weekday, unsigned int enabledAlarmsMask) const;
    void setTmFromTime();
    void setFromRtcTime(tm tm);
    void logSync(Settings::SyncSource source, int driftMs);
    tm startRtcSync();
    void startNtpSync();
    void startGpsSync();
    void onWifiConnectionFinished(bool success);

    enum RtcSync
    {
        SyncingFromRtc,
        SyncingToRtc,
        SyncDone
    };

    enum ExternalSync
    {
        Inactive,
        NtpWaitingForWifi,
        NtpInProgress,
        GpsInProgress,
    };

    CyclicCounter m_tickCount;
    Settings &m_settings;

    std::unique_ptr<Rtc> m_rtc; // As unique_ptr so that it can be easily disabled
    std::unique_ptr<Ntp> m_ntp;
    Gps m_gps;
    RtcSync m_rtcSync = SyncingFromRtc;
    int m_lastRtcSec;
    ExternalSync m_extSync = Inactive;

    SyncInfo m_syncInfo;

    DaylightSavingTime m_dst;
    time_t m_time = 0; // Current time as unix time, local (not UTC), not considering DST
    tm m_tm = {}; // Current time as tm, considering DST

    bool m_clockAdjusted = true;
    
};
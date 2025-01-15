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

// Structure for Weather information.  Probably belongs in Weather.h, not here.  //kdkWx
    struct WxInfo                                       //kdkWx
    {                                                   //kdkWx
        std::string conditions = "Clearsky";            //kdkWx
        float ctemp = 19.18;                            //kdkWx
        int pressure = 1022;                            //kdkWx
        int humidity = 61;                              //kdkWx
        float windSpeed = 11.5;                         //kdkWx
        int windDegree = 320;                           //kdkWx
        std::string windCardinal = "NNW";               //kdkWx
        uint64_t sunRise = 1736512199;                  //kdkWx
        uint64_t sunSet = 1736546826;                   //kdkWx
        uint64_t wxTimezone = -18000;                   //kdkWx
        std::string cityName = "Round Hill";            //kdkWx
        uint64_t wxDateTime = 1736517544;               //kdkWx
    };                                                  //kdkWx

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
    void syncWxNow();                                               // kdkWx
    void syncInfo(SyncInfo &info);
    void wxInfo(WxInfo &info);                                      //kdkWx

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
    void monitorWifiConnection();
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
    //  void logWeather(WxInfo);                    // kdkWx We will need one to update the weather info structure above.   
    tm startRtcSync();
    void startNtpSync();
    void startGpsSync();
    void startWxSync();                             // kdkWx We will need this once we start updating every 15min or 30 minutes

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
        WxWaitingForWifi,                           // kdkWx  Probably better to have it here, rather than a new enum.  Conflict with NTP
        WxInProgress                                // kdkWx  
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
    WxInfo m_wxInfo;                                                    // kdkWx  Points to our Weather structure above

    DaylightSavingTime m_dst;
    time_t m_time = 0; // Current time as unix time, local (not UTC), not considering DST
    tm m_tm = {}; // Current time as tm, considering DST

    bool m_clockAdjusted = true;
};
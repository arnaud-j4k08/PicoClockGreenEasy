#pragma once

#include "DaylightSavingTime.h"
#include "PicoClockHw/Gps.h"
#include "PicoClockHw/Rtc.h"
#include "PicoClockHw/Ntp.h"
#include "Settings.h"
#include "Utils/CyclicCounter.h"
#include "PicoClockHw/HttpRequest.h"            // kdkWx To be removed once Weather.cpp is up and running

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

// Structure for Weather information.                   //kdkWx  To be removed once Weather.cpp is up and running
    struct WxInfo                                       //kdkWx
    {                                                   //kdkWx
        std::string conditions = "No Data";             //kdkWx
        float ctemp = 0;                                //kdkWx
        int pressure = 0;                               //kdkWx
        int humidity = 0;                               //kdkWx
        float windSpeed = 0;                            //kdkWx
        int windDegree = 0;                             //kdkWx
        std::string windCardinal = "N";                 //kdkWx
        uint64_t sunRise = 0;                           //kdkWx
        uint64_t sunSet = 0;                            //kdkWx
        uint64_t wxTimezone = 0;                        //kdkWx
        std::string cityName = "No Data";               //kdkWx  Not currently supported by OpenWeatherMap 3.0 API
        uint64_t wxDateTime = 0;                        //kdkWx
    };                                                  //kdkWx

    // Beware that the object keeps a reference on settings, so it must exists at least as long as
    // the object.
    Clock(int tickPerSec, Settings &settings);

    void onWifiInited();
    void startSyncToRtc()
    {
        m_rtcSync = SyncingToRtc;
    }
    
    void tick(bool &clockAdjusted, Settings::AlarmMode &reachedAlarmMode); // kdkWx new boolean for &updateWeather
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
    void syncWxNow();                                               // kdkWx  Intermediate stub.  Will move to Weather.cpp
    void syncInfo(SyncInfo &info);
    void wxInfo(WxInfo &info);                                      // kdkWx  Used by WeatherInfo Will move to Weather.cpp

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
    void startWxSync();                             // kdkWx Called during Clock initialization, and periodic updates
                                                    // kdkWx Also part of Weather subMenu Action bind in ClockUi.  Moves to Weather.cpp

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
        WxWaitingForWifi,                           // kdkWx  Remove after creation of Weather.cpp
        WxInProgress                                // kdkWx  Remove after creation of Weather.cpp 
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
    WxInfo m_wxInfo;                                                         // kdkWx  pointer to WxInfo stucture Move to Weather.cpp

    std::string wifi_called_by;                                              // kdkWx  Used in onWifiConnectionFinished Remove 
    DaylightSavingTime m_dst;
    time_t m_time = 0; // Current time as unix time, local (not UTC), not considering DST
    tm m_tm = {}; // Current time as tm, considering DST

//    HttpRequest m_httpReq;                                                   // kdkWx Disable as we move to Weather.cpp
    void onRequestComplete(const std::string &content);                      // kdkWx Turned to stub before removal
    std::string extract(const std::string &json, const std::string &name);   // kdkWx Turned to stub before removal
    std::string extractStr(const std::string &json, const std::string &name); // kdkWx Turned to stub before removal
    std::string getCardinal(int degrees) const;                              // kdkWx Turned to stub before removal   

    bool m_clockAdjusted = true;
    bool m_wx = true;                                                        // kdkWx  Keep Boolean to test execution of Weather functions
};
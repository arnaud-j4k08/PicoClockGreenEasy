#include "Clock.h"
#include "Utils/Trace.h"
#include "PicoClockHw/Wifi.h"
#include "PicoClockHw/Platform.h"
#include <functional>

Clock::Clock(int tickPerSec, Settings &settings) : 
    m_tickCount(tickPerSec), 
    m_settings(settings),
    m_rtc(std::make_unique<Rtc>()),
    m_ntp(std::make_unique<Ntp>())
//    m_wx(std::make_unique<Weather>())     // kdkWx Remove after HttpRequest works
    // Todo kdkWx  I think that we will need a make_unique for Weather class.  Note that there isn't one for GPS.  Probably m_wx
{
    // Initialize m_time and m_tm from the RTC. It will be used for displaying time 
    // while waiting for the sync from the RTC to be finished.
    if (m_rtc != nullptr)
    {
        tm rtcTime = startRtcSync();
        setFromRtcTime(rtcTime);
    } else
    {
        TRACE << "No RTC available";
        m_rtc.release();
        m_rtcSync = SyncDone;
    }

    // Generate random time for the daily synchronization
    uint32_t randomNumber = Platform::randomNumber32();
    m_syncInfo.dailySyncHour = (randomNumber / 60) % 24;
    m_syncInfo.dailySyncMin = randomNumber % 60;

    TRACE << "Daily sync time: " << m_syncInfo.dailySyncHour << ":" << m_syncInfo.dailySyncMin;

    // Initialize GPS synchronization. NTP will be initialized later, as it requires the Wi-Fi to be
    // initialized.
    // kdkWx.  Weather will need to be later as well, as it depends on Wi-Fi.  
    using namespace std::placeholders;
    TRACE << "In Clock Clock, calling setOnCompleteCallback \n";                                // kdkWx
    m_httpReq.setOnCompleteCallback(std::bind(&Clock::onRequestComplete, this, _1));            // kdkWx
    TRACE << "In Clock Clock, after calling setOnCompleteCallback \n";                          // kdkWx
    m_gps.setTimeCallback(
        std::bind(&Clock::onExternalTimeReceived, this, _1, _2, Settings::SyncSource::Gps));
    m_gps.setTimeoutCallback([this]()
                             { 
            m_gps.setEnabled(false);
            m_extSync = Inactive; });

    // Start GPS reception if it is the selected sync source
    if (m_settings.get().syncSource == Settings::SyncSource::Gps)
        startGpsSync();
}

void Clock::onWifiInited()
{
    using namespace std::placeholders;
    if (m_ntp->init())
    {
        m_ntp->setTimeCallback(
            std::bind(&Clock::onExternalTimeReceived, this, _1, _2, Settings::SyncSource::Ntp));
        m_ntp->setFailCallback([this](Ntp::State reason)
                               { m_extSync = Inactive; });
        
        // Start NTP sync if it is the selected source
        if (m_settings.get().syncSource == Settings::SyncSource::Ntp)
            startNtpSync();
    } else
        m_ntp.release();
    // Todo.  kdkWx We will want to start Weather Sync here, if m_wx->init()   Ntp.cpp has an init function.  Create a copy of Ntp.cpp and call it Weather.cpp
    // For now, lets do the init, but not call the sync.  We will do the sync through SyncWxNow.cpp.
    // m_wx->init();          // kdkWx Both of these will go away once HttpRequest is working.  
    // startWxSync();       // kdkWx    
}

void Clock::syncNow()
{
    if (isSynchronizing())
    {
        TRACE << "Synchronization already in progress";
        return;
    }

    switch (m_settings.get().syncSource)
    {
        case Settings::SyncSource::Rtc:
            startRtcSync();
            break;
        case Settings::SyncSource::Ntp:
            startNtpSync();
            break;
        case Settings::SyncSource::Gps:
            startGpsSync();
            break;
    } 
}

void Clock::syncWxNow()                                     // kdkWx  Little stub, but mimics NTP syncing
{                                                           // kdkWx
    TRACE << "Made it to syncWxNow";                        // kdkWx
    if (isSynchronizing())                                  // kdkWx  Checks for SyncingFromRTC or ExternalSync not inactive
    {                                                       // kdkWx
        TRACE << "Synchronization already in progress";     // kdkWx  
        return;                                             // kdkWx
    }                                                       // kdkWx
        TRACE << "Past Check for Synchronizing";            // kdkWx
    startWxSync();                                          // kdkWx
}                                                           // kdkWx
                                                            // kdkWx
tm Clock::startRtcSync()
{
    tm rtcTime;
    if (m_rtc && m_rtc->read(rtcTime))
    {
        TRACE << "Set m_lastRtcSec to be able to detect when the second changes in the RTC";
        m_lastRtcSec = rtcTime.tm_sec;
        m_rtcSync = SyncingFromRtc;
        return rtcTime;
    } else
        return {};
}

void Clock::startNtpSync()
{
    TRACE << "startNtpSync";
    auto status = Wifi::linkStatus();
    TRACE << "Wifi link status: " <<Wifi::linkStatusToString(status);
    if (status != Wifi::Connected)
    {
        TRACE << "Wifi::connectAsync";
        Wifi::connectAsync();
        TRACE << "Wifi::connectAsync done";
        m_extSync = NtpWaitingForWifi;
    } else
    {
        TRACE << "Already connected";
        if (m_ntp)
        {
            m_ntp->startRequest();
            m_extSync = NtpInProgress;
        }
    }
}

void Clock::startWxSync()                                                       // kdkWx  This is also called from ClockUi submenu
{                                                                               // kdkWx  
    TRACE << "startWxsync";                                                     // kdkWx  
    auto status = Wifi::linkStatus();                                           // kdkWx  
    TRACE << "Wifi link status: " <<Wifi::linkStatusToString(status);           // kdkWx  
    if (status != Wifi::Connected)                                              // kdkWx  
    {                                                                           // kdkWx  
        TRACE << "Wifi::connectAsync";                                          // kdkWx  
        Wifi::connectAsync();                                                   // kdkWx  
        TRACE << "Wifi::connectAsync done";                                     // kdkWx  
        m_extSync = WxWaitingForWifi;                                           // kdkWx     New value in ExternalSync to avoid conflicts with NTP
    } else                                                                      // kdkWx  
    {                                                                           // kdkWx  
        TRACE << "Already connected";                                           // kdkWx  
        if (m_wx)                                                               // kdkWx  was uniqueptr, now boolean.  Are we doing Weather?  
        {                                                                       // kdkWx 
        TRACE << "In Clock startWxSync, calling m_httpReq start \n";            // kdkWx  
            m_httpReq.start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL); 
        TRACE << "In Clock startWxSync, after calling m_httpReq start \n";      // kdkWx      
            m_extSync = WxInProgress;                                           // kdkWx Update sync in progress for other processes
        //    m_wx->startRequest();                                             // kdkWx Delete when HttpRequest works
        }                                                                       // kdkWx  
    }                                                                           // kdkWx  
}                                                                               // kdkWx  


void Clock::onRequestComplete(const std::string &content)                       // kdkWx Entire section moved from Weather.cpp
{
    TRACE << "In Clock onRequestComplete: \n";
    std::cout << m_httpReq.content() <<std::endl;
    // json += m_httpReq.content(); // kdkWx don't know why we need to append to the string, instead of assign
    json = m_httpReq.content();
    m_extSync = Inactive;           // allow new synchronizations
    if (json.size() < 400)
        return;

    tempstr2 = Clock::extractStr(json, "description");
    m_wxInfo.conditions = tempstr2;
    std::cout <<"conditions: " <<tempstr2  <<std::endl;
//    std::cout <<"conditions: " <<HttpRequest::extractStr(json, "description") <<std::endl;
    tempstr2 = Clock::extract(json, "temp");
    m_wxInfo.ctemp = std::stof(tempstr2);
    std::cout <<"ctemp: " <<tempstr2 <<std::endl;
//    std::cout <<"ctemp: " <<HttpRequest::extract(json, "temp") <<std::endl;
    tempstr2 = Clock::extract(json, "pressure");
    m_wxInfo.pressure = std::stoi(tempstr2);
    std::cout <<"pressure: " <<tempstr2 <<std::endl;
//    std::cout <<"pressure: " <<HttpRequest::extract(json, "pressure") <<std::endl;
    tempstr2 = Clock::extract(json, "humidity");
    m_wxInfo.humidity = std::stoi(tempstr2);
    std::cout <<"humidity: " <<tempstr2 <<std::endl;
//    std::cout <<"humidity: " <<HttpRequest::extract(json, "humidity") <<std::endl;
    tempstr2 = Clock::extract(json, "speed");
    m_wxInfo.windSpeed = std::stof(tempstr2);
    std::cout <<"windSpeed: " <<tempstr2 <<std::endl;       
//    std::cout <<"windSpeed: " <<HttpRequest::extract(json, "speed") <<std::endl;
    tempstr2 = Clock::extract(json, "deg");
    m_wxInfo.windDegree = std::stoi(tempstr2);
    tempInt = std::stoi(tempstr2);
    std::cout <<"windDegree: " <<tempstr2 <<std::endl;
//    std::cout <<"windDegree: " <<HttpRequest::extract(json, "deg") <<std::endl;
    tempstr2 = "BAD";
    tempstr2 = Clock::getCardinal(tempInt);
    m_wxInfo.windCardinal = tempstr2;
    std::cout <<"windCardinal: " <<tempstr2 <<std::endl;
    // calculate and display windCardinal
    tempstr2 = Clock::extract(json, "sunrise");
    m_wxInfo.sunRise = std::stoull(tempstr2);
    std::cout <<"sunRise: " <<tempstr2 <<std::endl;
//    std::cout <<"sunRise: " <<HttpRequest::extract(json, "sunrise") <<std::endl;
    tempstr2 = Clock::extract(json, "sunset");
    m_wxInfo.sunSet = std::stoull(tempstr2);
    std::cout <<"sunSet: " <<tempstr2 <<std::endl;
//    std::cout <<"sunSet: " <<HttpRequest::extract(json, "sunset") <<std::endl;
    tempstr2 =Clock::extract(json, "timezone");
    m_wxInfo.wxTimezone = std::stoull(tempstr2);
    std::cout <<"wxTimeZone: " <<tempstr2 <<std::endl;
//    std::cout <<"wxTimeZone: " <<HttpRequest::extract(json, "timezone") <<std::endl;
    tempstr2 = Clock::extractStr(json, "name");
    m_wxInfo.cityName = tempstr2;
    std::cout <<"cityName: " <<tempstr2 <<std::endl;
//    std::cout <<"cityName: " <<HttpRequest::extractStr(json, "name") <<std::endl;
    tempstr2 = Clock::extract(json, "dt");
    m_wxInfo.wxDateTime = std::stoull(tempstr2);
    std::cout <<"wxDateTime: " <<tempstr2 <<std::endl;
//    std::cout <<"wxDateTime: " <<HttpRequest::extract(json, "dt") <<std::endl;

}

std::string Clock::extract(const std::string &json, const std::string &name)    // kdkWx Entire section moved from Weather.cpp
{
    std::string prefix = "\"" + name + "\":";
    auto prefixPos = json.find(prefix);
    if (prefixPos != std::string::npos)
    {
        auto beginPos = prefixPos + prefix.size();
        auto endPos = json.find(",", beginPos);
        auto endPosbrace = json.find("}", beginPos);
        if (endPosbrace != std::string::npos && endPos > endPosbrace)
        { 
            endPos = endPosbrace;
        }
        return json.substr(beginPos, endPos - beginPos);
    } else
        return "";
}

std::string Clock::extractStr(const std::string &json, const std::string &name)   // kdkWx Entire section moved from Weather.cpp
{
    std::string value = extract(json, name);
    return value.substr(1, value.size() - 2);
}

std::string Clock::getCardinal(int degrees) const                                 // kdkWx Entire section moved from Weather.cpp
{

    if (degrees < 11) return "N";
    if (degrees < 34) return "NNE";
    if (degrees < 56) return "NE";
    if (degrees < 79) return "ENE";
    if (degrees < 101) return "E";
    if (degrees < 123) return "ESE";
    if (degrees < 146) return "SE";
    if (degrees < 169) return "SSE";
    if (degrees < 191) return "S";
    if (degrees < 214) return "SSW";
    if (degrees < 236) return "SW";
    if (degrees < 259) return "WSW";
    if (degrees < 282) return "W";
    if (degrees < 304) return "WNW";
    if (degrees < 327) return "NW";
    if (degrees < 349) return "NNW";
    return "N";

}

void Clock::startGpsSync()
{
    m_gps.setEnabled(true);
    m_extSync = GpsInProgress;
}

void Clock::onExternalTimeReceived(time_t utcTime, uint32_t ms, Settings::SyncSource source)
{
    TRACE << "Received external UTC time:" << utcTime <<"." <<ms;
    time_t newTime = utcTime + UTC_OFFSET * 60 * 60;
    int drift = 
        (m_time * 1000 + m_tickCount * 1000 / m_tickCount.wrapValue() - newTime * 1000 - ms);
    m_time = newTime;
    setTmFromTime();
    logSync(source, drift);
    m_tickCount = ms * m_tickCount.wrapValue() / 1000;
    m_clockAdjusted = true;

    m_extSync = Inactive;

    // Now that we got the time from outside, plan RTC sync at the next second change.
    m_rtcSync = SyncingToRtc;

    // Disable GPS as it is no longer needed, in case the clock was synchronized from it.
    m_gps.setEnabled(false);
}

void Clock::tick(bool &clockAdjusted, Settings::AlarmMode &reachedAlarmMode)
{
    clockAdjusted = false;
    reachedAlarmMode = Settings::AlarmMode::Off;

    if (m_tickCount.increment())
    {
        m_time++;
        setTmFromTime();
    }

    if (m_rtc && m_rtcSync == SyncingFromRtc) // RTC available and synchronizing with it?
    {
//        TRACE << "Synchronizing with RTC";
        tm rtcTime;
        if (m_rtc->read(rtcTime))
        {
            if (rtcTime.tm_sec != m_lastRtcSec)
            {
                TRACE << "Sync from RTC done";

                // The second just changed in the RTC, synchronize.
                setFromRtcTime(rtcTime);
                m_rtcSync = SyncDone;
            }
        } else
        {
            // RTC read failed, give up with synchronization
            m_rtcSync = SyncDone;
        }
    } else
    {
        // Sync to RTC if needed
        if (m_tickCount == 0 && m_rtc && m_rtcSync == SyncingToRtc)
        {
            TRACE << "Set RTC";

            // To avoid ambiguity, save the time without DST consideration into the RTC. Thus,
            // on the next start, m_dst will be able to determine if DST is active only by 
            // looking at the time and date.
            tm tm = *localtime(&m_time);

            if (m_rtc->write(tm))
                m_rtcSync = SyncDone;
        }
    }

    monitorWifiConnection();

    // Handle events to be checked on every minute.
    if (m_tickCount == 0 && m_tm.tm_sec == 0)
    {
        reachedAlarmMode = checkIfAlarmReached();

        // Perform the daily synchronization if the time is reached.
        if (m_tm.tm_min == m_syncInfo.dailySyncMin && m_tm.tm_hour == m_syncInfo.dailySyncHour)
            syncNow();
    }

    if (m_clockAdjusted)
    {
        m_clockAdjusted = false;
        clockAdjusted = true;
    } 
}

void Clock::monitorWifiConnection()
{
    if (m_extSync == NtpWaitingForWifi)
    {
        auto status = Wifi::linkStatus();
        switch (status)
        {
            case Wifi::Connecting:
            case Wifi::NoIp:
                // Continue waiting for connection
                break;
            case Wifi::Connected:
                if (m_ntp)
                {
                    TRACE << "Wifi connected, start NTP request";
                    m_ntp->startRequest();
                    TRACE << "Request started";
                    m_extSync = NtpInProgress;
                } else
                    m_extSync = Inactive;
                break;
            default:
                TRACE << "Connection failed";
                m_extSync = Inactive;
        }
    }

    if (m_extSync == WxWaitingForWifi)                                              // kdkWx  Duplicated for Weather
    {                                                                               // kdkWx
        auto status = Wifi::linkStatus();                                           // kdkWx
        switch (status)                                                             // kdkWx
        {                                                                           // kdkWx
            case Wifi::Connecting:                                                  // kdkWx
            case Wifi::NoIp:                                                        // kdkWx
                // Continue waiting for connection                                  // kdkWx
                break;                                                              // kdkWx
            case Wifi::Connected:                                                   // kdkWx
              if (m_wx)                                                             // kdkWx  Was UniquePtr, may be boolean or removed
                {                                                                   // kdkWx 
                 TRACE << "In Clock monitorWifiConnection, calling m_httpReq start \n";                                                                   // kdkWx
                //    TRACE << "Wifi connected, start Weather request";             // kdkWx
                //    m_wx->startRequest();
                      m_httpReq.start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL); 
                    TRACE << "In Clock monitorWifiConnection, after calling m_httpReq start \n";                                           // kdkWx
                //    TRACE << "Weather Request started";                           // kdkWx
                      m_extSync = WxInProgress;
                } else                                                              // kdkWx
                    m_extSync = Inactive;                                           // kdkWx
                break;                                                              // kdkWx     
            default:                                                                // kdkWx  
                TRACE << "Connection failed";                                       // kdkWx  
                m_extSync = Inactive;                                               // kdkWx  
        }                                                                           // kdkWx  
    }                                                                               // kdkWx  
}

Settings::AlarmMode Clock::checkIfAlarmReached()
{
    Settings::AlarmMode reachedAlarmMode = Settings::AlarmMode::Off;
    const struct Settings::Alarm *reachedAlarm = nullptr;
    if (alarmReached(Alarm1))
    {
        reachedAlarm = &m_settings.get().alarm1;
        reachedAlarmMode = m_settings.get().alarm1.mode;
    } else if (alarmReached(Alarm2))
    {
        reachedAlarm = &m_settings.get().alarm2;
        reachedAlarmMode = m_settings.get().alarm2.mode;
    }

    if (reachedAlarm != nullptr)
    {
        // Disable the alarm if it has to ring only once.
        if (reachedAlarm->ringsOnce())
        {
            struct Settings::Alarm &alarm = 
                reachedAlarm == 
                    &m_settings.get().alarm1 ? m_settings.modify().alarm1 : m_settings.modify().alarm2;

            alarm.mode = Settings::AlarmMode::Off;
        }

        if (m_settings.get().skipNextAlarm)
        {
            // This alarm must be skipped. Disable the "skip next alarm" function and report 
            // that no alarm was reached.
            m_settings.modify().skipNextAlarm = false;
            reachedAlarmMode = Settings::AlarmMode::Off;
        }
    }

    return reachedAlarmMode;
}

void Clock::setTmFromTime()
{
    time_t timeConsideringDst = m_dst.considerDst(m_time);
    m_tm = *localtime(&timeConsideringDst);
    // TRACE << "It is" << m_tm;
}

void Clock::set(const tm &tm)
{
    TRACE << "Set clock to" << tm;
    m_tm = tm;

    // If DST is active, unapply it so that the time stays as it was set by the user, as the DST 
    // offset will be readded each time setTmFromTime() is called.
    m_time = m_dst.unconsiderDst(mktime(&m_tm));

    m_clockAdjusted = true;
}

// tm is passed by copy so that its address can be passed to mktime
void Clock::setFromRtcTime(tm tm)
{
    time_t newTime = mktime(&tm);
    int drift =
        (m_time * m_tickCount.wrapValue() + m_tickCount - newTime * m_tickCount.wrapValue()) *
        1000 /
        m_tickCount.wrapValue();
    m_time = newTime;
    m_tickCount = 0;
    setTmFromTime();
    logSync(Settings::SyncSource::Rtc, drift);

    m_clockAdjusted = true;
}

void Clock::logSync(Settings::SyncSource source, int driftMs)
{
    m_syncInfo.lastSyncTm = m_tm;
    m_syncInfo.lastSyncSource = source;
    m_syncInfo.lastSyncDriftMs = driftMs;
}

void Clock::syncInfo(SyncInfo &info)
{
    info = m_syncInfo;
}

// Superceded by onRequestComplete .  Probably Can Delete  kdkWx
void Clock::logWeather(Clock::WxInfo &info)                   // kdkWx
{                                                             // kdkWx
    m_wxInfo.conditions = info.conditions;                    // kdkWx
    m_wxInfo.ctemp = info.ctemp;                              // kdkWx
    m_wxInfo.pressure = info.pressure;                        // kdkWx
    m_wxInfo.humidity = info.humidity;                        // kdkWx
    m_wxInfo.windSpeed = info.windSpeed;                      // kdkWx
    m_wxInfo.windDegree = info.windDegree;                    // kdkWx
    m_wxInfo.windCardinal = info.windCardinal;                // kdkWx
    m_wxInfo.sunRise = info.sunRise;                          // kdkWx
    m_wxInfo.sunSet = info.sunSet;                            // kdkWx
    m_wxInfo.wxTimezone = info.wxTimezone;                    // kdkWx
    m_wxInfo.cityName = info.cityName;                        // kdkWx
    m_wxInfo.wxDateTime = info.wxDateTime;                     // kdkWx
// or                                                           // kdkWx
//    can we m_wxInfo = info  ?   is it just that easy?         // kdkWx
//                                                              // kdkWx
}                                                               // kdkWx
//                                                              // kdkWx
//                                                              // kdkWx
void Clock::wxInfo(WxInfo &info)                                // kdkWx  Used by WeatherInfo.cpp
{                                                               // kdkWx
    info = m_wxInfo;                                            // kdkWx  m_wxInfo is a pointer to structure in Clock.h 
}                                                               // kdkWx


bool Clock::nextAlarm(int &weekday, int &hour, int &min) const
{
    // Keep track of which alarms would be enabled, as alarms that ring only once disable 
    // themselves.
    unsigned int enabledAlarmsMask = 0;
    if (alarm(Alarm1).mode != Settings::AlarmMode::Off)
        enabledAlarmsMask |= 1;
    if (alarm(Alarm2).mode != Settings::AlarmMode::Off)
        enabledAlarmsMask |= 2;

    // Figure out what is the next alarm after now
    Time time;
    if (!nextAlarmAfter(
            m_tm.tm_wday, {m_tm.tm_hour, m_tm.tm_min}, weekday, time, enabledAlarmsMask))
        return false; // No alarm enabled

    // If this alarm will be skipped, get the next alarm after it
    if (m_settings.get().skipNextAlarm)
    {
        if (!nextAlarmAfter(weekday, time, weekday, time, enabledAlarmsMask))
            return false; // No other alarm enabled
    }

    // deliver the result
    hour = time.hour;
    min = time.min;
    return true;
}

bool Clock::nextAlarmAfter(
        int startWeekday, 
        const Time &startTime, 
        int &weekday, 
        Time &time, 
        unsigned int &enabledAlarmsMask) const
{
    // Check if there is an alarm later on the start day
    Time t1 = alarmTimeAtDay(Alarm1, startWeekday, enabledAlarmsMask);
    if (t1 <= startTime)
    {
        TRACE << "Alarm 1 already passed today";
        t1 = Time();
    }
    Time t2 = alarmTimeAtDay(Alarm2, startWeekday, enabledAlarmsMask);
    if (t2 <= startTime)
    {
        TRACE << "Alarm 2 already passed today";
        t2 = Time();
    }

    // Return the earliest reacheable alarm on the start day if one was found.
    if (earliestAlarm(t1, t2, time, enabledAlarmsMask))
    {
        weekday = startWeekday;
        return true;
    }

    // Check the following 7 weekdays, wrapping around to the same weekday in 7 days
    CyclicCounter currentWeekDay{7, startWeekday};
    for (int i = 0; i < 7; i++)
    {
        currentWeekDay.increment();
        t1 = alarmTimeAtDay(Alarm1, currentWeekDay, enabledAlarmsMask);
        t2 = alarmTimeAtDay(Alarm2, currentWeekDay, enabledAlarmsMask);

        // Return the earliest reacheable alarm on this day if one was found.
        if (earliestAlarm(t1, t2, time, enabledAlarmsMask))
        {
            weekday = currentWeekDay;
            return true;
        }
    }

    // No next alarm found
    weekday = -1;
    return false;
}

bool Clock::earliestAlarm(
    const Time &alarm1Time, 
    const Time &alarm2Time, 
    Time &earliestTime,  
    unsigned int &enabledAlarmsMask) const
{
    if (alarm1Time.isValid() && alarm1Time <= alarm2Time)
    {
        earliestTime = alarm1Time;
        if (alarm(Alarm1).ringsOnce())
        {
            // This alarm would disable itself, even if "skip next alarm" is used
            enabledAlarmsMask &= ~1;
        }
        return true;
    }

    if (alarm2Time.isValid())
    {
        earliestTime = alarm2Time;
        if (alarm(Alarm2).ringsOnce())
        {
            // This alarm would disable itself, even if "skip next alarm" is used
            enabledAlarmsMask &= ~2;
        }
        return true;
    }

    return false;
}

Clock::Time Clock::alarmTimeAtDay(
    AlarmId alarmId, int weekday, unsigned int enabledAlarmsMask) const
{
    if ((enabledAlarmsMask & (1<<alarmId)) &&
        (alarm(alarmId).ringsOnce() || alarm(alarmId).enabledOnWeekDay(weekday)))
    {
        TRACE 
            << "Alarm" 
            << alarmId + 1 
            << "will ring on day" 
            << weekday 
            << "at" 
            << alarm(alarmId).hour 
            << ":" 
            << alarm(alarmId).min;
        return {alarm(alarmId).hour, alarm(alarmId).min};
    }
    else
        return Time();
}

bool Clock::alarmReached(AlarmId id) const
{
    const Settings::Alarm &al = alarm(id);
    return 
        m_tm.tm_min == al.min && 
        m_tm.tm_hour == al.hour && 
        (al.ringsOnce() || al.enabledOnWeekDay(m_tm.tm_wday)) && 
        al.mode != Settings::AlarmMode::Off;
}

const Settings::Alarm &Clock::alarm(AlarmId id) const
{
    if (id == Alarm1)
        return m_settings.get().alarm1;
    else
        return m_settings.get().alarm2;
}

bool Clock::Time::operator <(const Time &other) const
{
    if (hour < other.hour)
        return true;
    if (hour > other.hour)
        return false;
    return min < other.min;
}

bool Clock::Time::operator <=(const Time &other) const
{
    return !(other < *this);
}

bool Clock::Time::isValid() const
{
    return hour >= 0 && hour < 24 && min >= 0 && min < 60;
}

#pragma once

#include <string>

enum class TextId
{
    // For menus
    Alarms = 0,
    Options,
    Exit,

    // Wifi status. These ids must match those in Wifi::Status
    Unknown,
    OK,
    NotAvailable,
    Down,
    Connecting,
    NoIp,
    Connected,
    ConnectionFailed,
    NoNetworkFound,
    AuthenticationFailed,

    // For alarms
    NextColon,
    AlarmShortened1Colon,
    AlarmShortened2Colon,
    Loud,
    Gradual,
    SkipNextAlarmColon,
    Once,
    Weekly,

    // For options
    AutoLightColon,
    TimeFormatColon,
    Format24h,
    Format12h,
    DateFormatColon,
    HourlyChimeColon,
    AutoScrollColon,
    BrightnessColon,
    BrightnessDarkColon,
    BrightnessDimColon,
    BrightnessBrightColon,
    On,
    Off,
    Day,

    // Stopwatch and countdown
    Stopwatch,
    Reset,
    Countdown,
    Set,

    // Date formats. Must match the order of Settings::DateFormat.
    MonthDashDay,
    MonthSlashDay,
    DayDashMonth,
    DaySlashMonth,
    DayDotMonth,

    // Sync menu
    Sync,
    SourceColon,
    SyncNow,
    DailySyncTimeColon,
    LastSyncColon,
    LastDriftColon,
    WifiColon,

    // Sync source. Must match Settings::SyncSource
    Rtc,
    Ntp,
    Gps,

    // Weather related Text.  Associated with WxInfo structure kdkWx
    Weather,        // kdkWx
    Conditions,     // kdkWx
    Temperature,    // kdkWx
    Pressure,       // kdkWx
    Humidity,       // kdkWx
    WindSpeed,      // kdkWx
    WindDir,        // kdkWx
    WindCard,       // kdkWx
    Sunrise,        // kdkWx
    Sunset,         // kdkWx
    CityName,       // kdkWx
    SyncWxNow,      // kdkWx
    LastUpdate,     // kdkWx


    TextCount
};

std::string uiText(TextId id);
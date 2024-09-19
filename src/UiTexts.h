#pragma once

#include <string>

enum class TextId
{
    // For menus
    Alarms = 0,
    WifiColon,
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

    // For options
    AutoLightColon,
    FormatColon,
    Format24h,
    Format12h,
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

    TextCount
};

std::string uiText(TextId id);
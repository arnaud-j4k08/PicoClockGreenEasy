#pragma once

#include <string>

enum Language
{
    English = 0,
    French = 1,
    LanguageCount
};

enum class TextId
{
    // Main menu
    Alarms = 0,
    Tools,
    Sync,
    Options,
    Exit,

    // Alarms
    NextColon,
    AlarmShortened1Colon,
    AlarmShortened2Colon,
    VolumeColon,
    TrackColon,
    Random,
    SkipNextAlarmColon,
    Once,
    Weekly,

    // Options
    AutoLightColon,
    TimeFormatColon,
    Format24h,
    Format12h,
    DateFormatColon,
    HourlyChimeColon,
    AutoScrollColon,
    DigitAnimation,
    BrightnessColon,
    BrightnessDarkColon,
    BrightnessDimColon,
    BrightnessBrightColon,
    On,
    Off,
    ChimeVolumeColon,

    // Tools menu, stopwatch and countdown
    Flashlight,
    Stopwatch,
    Reset,
    Countdown,
    Set,

    // Sync menu
    SourceColon,
    SyncNow,
    DailySyncTimeColon,
    LastSyncColon,
    LastDriftColon,
    WifiColon,

    Count
};

template <typename Enum>
extern const char *g_textTable[][LanguageCount];

// Return the text associated with the given enum item in the current language. A table for the enum
// must be defined in UiTexts.cpp.
template <typename Enum> std::string uiText(Enum id)
{
    return g_textTable<Enum>[static_cast<unsigned int>(id)][LANGUAGE];
}
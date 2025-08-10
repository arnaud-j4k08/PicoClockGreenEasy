#pragma once

#include <cstdint>

class Settings
{
public:
    enum class HourlyChimeMode : uint8_t
    {
        Off = 0,
        Beep = 1,
        BeepOnDayLight,
        Sound,
        SoundOnDayLight,

        Count
    };

    enum class AlarmMode
    {
        Off = 0,
        Gradual,
        Loud,
        Music,
        Count
    };

    enum class DateFormat
    {
        MonthDashDay,
        MonthSlashDay,
        DayDashMonth,
        DaySlashMonth,
        DayDotMonth,
        Count
    };

    enum class SyncSource
    {
        Rtc,
        Ntp,
        Gps,
        Count
    };

    struct Alarm
    {
        AlarmMode mode = AlarmMode::Off;
        int hour = 6;
        int min = 0;
        
        // Set to Monday to Friday by default. A value of 0 make the alarm ring only once when 
        // hour:min is reached.
        uint8_t weekDayBits = 0x3E;

        bool ringsOnce() const // Does the alarm ring only once?
        {
            return weekDayBits == 0;
        }

        bool enabledOnWeekDay(int weekDay) const
        {
            return weekDayBits & (1 << weekDay);
        }
    };

    struct AlarmMusic
    {
        int track = 0; // 0 means random track
        int volume = 30;
    };

    struct Values
    {
        int8_t function = 1; // Time with HourMinBar style by default
        bool autoScroll = false;
        bool useCelsius = true;
        bool format24h = true;
        HourlyChimeMode hourlyChimeMode = HourlyChimeMode::Off; 
        
        // Determines if the brightness is automatically adjusted based on ambient light. Changing 
        // this requires a call to considerManualBrightness() in case the display brightness needs
        // to be updated.
        bool autoLight = true; 
        
        Alarm alarm1;
        Alarm alarm2;
        bool skipNextAlarm = false;
        int countdownStartMin = 1;
        int countdownStartSec = 0;

        // Brightness when autoLight is disabled. This is not considered continuously for the 
        // display brightness, so considerManualBrightness() needs to be called after changing it.
        int manualBrightness = 100;

        int brightnessDark = -20;
        int brightnessDim = 55;
        int brightnessBright = 100;
        DateFormat dateFormat = DateFormat::MonthDashDay;
        SyncSource syncSource = SyncSource::Rtc;
        int flashlightBrightness = 50;
        int chimeSoundVolume = 30;
        AlarmMusic alarmMusic[2];
        bool digitAnimation = true;
    };

    Settings();

    // Get read only access to settings values
    const Values &get() const
    {
        return m_values;
    }

    // Get write access to settings values and schedule saving to flash memory
    Values &modify();

private:
    Values m_values;
};
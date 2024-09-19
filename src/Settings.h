#pragma once

#include <cstdint>

class Settings
{
public:
    enum class HourlyChimeMode : uint8_t
    {
        Off = 0,
        On = 1,
        OnDayLight,
        Count
    };

    enum class AlarmMode
    {
        Off = 0,
        Gradual,
        Loud,
        Count
    };

    struct Alarm
    {
        AlarmMode mode = AlarmMode::Off;
        int hour = 6;
        int min = 0;
        uint8_t weekDayBits = 0x3E; // Monday to Friday

        bool enabledOnWeekDay(int weekDay) const
        {
            return weekDayBits & (1 << weekDay);
        }
    };

    struct Values
    {
        int8_t function = 1; // Time with HourMinBar style by default
        bool autoScroll = false;
        bool useCelsius = true;
        bool format24h = true;
        HourlyChimeMode hourlyChime = HourlyChimeMode::Off; 
        bool autoLight = true; 
        Alarm alarm1;
        Alarm alarm2;
        bool skipNextAlarm = false;
        int countdownStartMin = 1;
        int countdownStartSec = 0;
        int manualBrightness = 100;
        int brightnessDark = -20;
        int brightnessDim = 55;
        int brightnessBright = 100;
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
#pragma once
//  This is a copy of SyncInfo that has been modified to display Weather Information  kdkWx
#include "AbstractFunction.h"
#include "PicoClockHw/Weather.h"
#include <time.h>

class WeatherInfo : public AbstractFunction
{
public:
    enum Entry
    {
        WxConditions,
        WxTemperature,
        WxPressure,
        WxHumidity,
        WxWind,
        WxWindDirection,
        WxSunrise,
        WxSunset,
        WxName,  // Not currently supported in OpenWeatherMap 3.0 API
        WxDateTime,
    };
    WeatherInfo(ClockUi *clockUi, Entry entry, Weather *weather) : AbstractFunction(clockUi), m_entry(entry)
    {}

private:
    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    int valueCount() const override
    {
        return 1; // Nothing to edit, the information is read only
    }
    std::string timeToString(int hour, int min, bool &morning) const;
    std::string timeToString(const tm &tm, bool &morning) const;
    std::string dateToString(const tm &tm) const;

    Entry m_entry;

};
#pragma once
//  This is a copy of SyncInfo that has been modified to display Weather Information  kdkWx
#include "AbstractFunction.h"
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
        WxName,
        WxDateTime,
    };
    WeatherInfo(ClockUi *clockUi, Entry entry) : AbstractFunction(clockUi), m_entry(entry)
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

    // My local time variables for calculating weather times.  Trying to avoid memory leak.
    time_t WxTime = 0; // UTC time as unix time, local,  not considering DST
    tm m_Wxtm = {}; // Current time as tm, not considering DST
};
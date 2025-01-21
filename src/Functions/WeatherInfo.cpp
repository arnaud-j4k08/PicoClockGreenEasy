#include "WeatherInfo.h"
#include "Clock.h"
#include "UiTexts.h"
#include <sstream>

namespace
{
    std::string to2DigitsString(int number)
    {
        std::string s = std::to_string(number);
        if (s.size() == 1)
            s = '0' + s;
        return s;
    }
}

void WeatherInfo::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) 
{
    Clock::WxInfo info;
    clock().wxInfo(info);
    std::string sourceText = 
    uiText(static_cast<TextId>(
    //    static_cast<int>(TextId::Rtc) + static_cast<int>(info.lastSyncSource)));  // I don't know what this was doing. 
        static_cast<int>(TextId::Rtc)));

    std::string text;
    bool morning;
    switch (m_entry)
    {
        case WxConditions:
            text = 
                uiText(TextId::Conditions) + 
                (info.conditions);
            break;
        case WxTemperature:
            char tempString[6];
            sprintf(tempString, "%5.2f", info.ctemp);
            
            text = 
                uiText(TextId::Temperature) + tempString + " F";

            if (settings().useCelsius)
                frame.putIndicator(Bitmap::C, true);
            else
                frame.putIndicator(Bitmap::F, true);   
//                std::to_string(info.ctemp);
            break;  
         case WxPressure:
            text = 
                uiText(TextId::Pressure) + 
                std::to_string(info.pressure);
            break; 
        case WxHumidity:
            text = 
                uiText(TextId::Humidity) + 
                std::to_string(info.humidity) + " %";
            break; 
        case WxWind:
            char tempString2[6];
            sprintf(tempString2, "%5.2f", info.windSpeed);
            text = 
                uiText(TextId::WindSpeed) + tempString2 + " MPH";
//                std::to_string(info.windSpeed);
            break; 
        case WxWindDirection:
            text = 
//              uiText(TextId::WindDir) + getCardinal(info.windDegree) + " (" + std::to_string(info.windDegree) + ")";
                uiText(TextId::WindDir) + info.windCardinal + " (" + std::to_string(info.windDegree) + ")";
//                std::to_string(info.windSpeed);
            break;          
        case WxSunrise:   
            WxTime = info.sunRise + info.wxTimezone;  //  Remove the UTC offset in seconds.  This finally gives us local time
            m_Wxtm = *localtime(&WxTime); 

            text =
                uiText(TextId::Sunrise) + timeToString(m_Wxtm, morning);
            putAmPmIndicators(frame, morning);
            break;
        case WxDateTime:   
            WxTime = info.wxDateTime + info.wxTimezone;  //  Remove the UTC offset in seconds.  This finally gives local time
            m_Wxtm = *localtime(&WxTime); 

            text =
                uiText(TextId::LastSyncColon) + timeToString(m_Wxtm, morning) +
                " " + dateToString(m_Wxtm); 
            putAmPmIndicators(frame, morning);

            break;            
        case WxSunset:   
            WxTime = info.sunSet + info.wxTimezone;  //  Remove the UTC offset in seconds.  This finally gives us local time
            m_Wxtm = *localtime(&WxTime); 

            text =
                uiText(TextId::Sunset) + timeToString(m_Wxtm, morning);
            putAmPmIndicators(frame, morning);
            break;
        case WxName:
            text = 
                uiText(TextId::CityName) + 
                (info.cityName);
            break;
    }

    renderScrollingText(frame, fullRefresh, text);
}

std::string WeatherInfo::timeToString(int hour, int min, bool &morning) const
{
    int displayedHour;
    convertHour(hour, displayedHour, morning);

    return std::to_string(displayedHour) + ":" + to2DigitsString(min);
}

std::string WeatherInfo::timeToString(const tm &tm, bool &morning) const
{
    int displayedHour;
    convertHour(tm.tm_hour, displayedHour, morning);

    return 
        std::to_string(displayedHour) + 
        ":" + 
        to2DigitsString(tm.tm_min) + 
        ":" + 
        to2DigitsString(tm.tm_sec);
}

std::string WeatherInfo::dateToString(const tm &tm) const
{
    std::string month = to2DigitsString(tm.tm_mon + 1);
    std::string day = to2DigitsString(tm.tm_mday);

    switch(settings().dateFormat)
    {
        case Settings::DateFormat::MonthDashDay:
            return month + "-" + day;
        case Settings::DateFormat::MonthSlashDay:
            return month + "/" + day;
        case Settings::DateFormat::DayDashMonth:
            return day + "-" + month;
        case Settings::DateFormat::DaySlashMonth:
            return day + "/" + month;
        case Settings::DateFormat::DayDotMonth:
            return day + "." + month;
    }

    return "";
}

std::string WeatherInfo::getCardinal(int degrees) const   // This is currently a lot to do each second.  If it is done on sync, not so much kdkWx
{
//    if (degrees < 22) return "N";
//    if (degrees < 67) return "NE";
//    if (degrees < 113) return "E";
//    if (degrees < 158) return "SE";
//    if (degrees < 202) return "S";
//    if (degrees < 248) return "SW";
//    if (degrees < 293) return "W";
//    if (degrees < 338) return "NW";
//    return "N";

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

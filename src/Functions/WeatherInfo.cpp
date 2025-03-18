#include "WeatherInfo.h"
#include "Clock.h"
#include "UiTexts.h"
#include <sstream>

namespace
{
    // TODO: share implementation with the other one in SyncInfo.cpp
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
    Weather::WeatherInfo info;
    m_weather->weatherInfo(info);

    std::string text;
    bool morning;
    switch (m_entry)
    {
        case Conditions:
            text = 
                uiText(TextId::Conditions) + 
                (info.conditions);
            break;
        case Temperature:
            char tempString[6];
            sprintf(tempString, "%5.2f", info.ctemp);
            
            text = 
                uiText(TextId::Temperature) + tempString ;

            if (settings().useCelsius)
                frame.putIndicator(Bitmap::C, true);
            else
                frame.putIndicator(Bitmap::F, true);   
            break;  
         case Pressure:
            text = 
                uiText(TextId::Pressure) + 
                std::to_string(info.pressure);
            break; 
        case Humidity:
            text = 
                uiText(TextId::Humidity) + 
                std::to_string(info.humidity) + " %";
            break; 
        case Wind:
            char tempString2[6];
            sprintf(tempString2, "%5.2f", info.windSpeed);
            if (settings().useCelsius)
                {sprintf(tempString2, "%5.2f", info.windSpeed * 3.6);  // Convert from metres/sec to Km/Hour
                text = 
                uiText(TextId::WindSpeed) + tempString2 + " KMH";
                }
            else
                text = 
                uiText(TextId::WindSpeed) + tempString2 + " MPH";
            break; 
        case WindDirection:
            text = 
                uiText(TextId::WindDir) + info.windCardinal + " (" + std::to_string(info.windDegree) + ")";
            break;          
        case Sunrise:
        {
            time_t weatherTime = info.sunRise + info.timezone;  //  Remove the UTC offset in seconds.  This gives us local time
            tm weatherTm = *localtime(&weatherTime); 

            text = uiText(TextId::Sunrise) + timeToString(weatherTm, morning);
            putAmPmIndicators(frame, morning);
            break;
        }
        case DateTime:
        {
            time_t weatherTime = info.dateTime + info.timezone;  //  Remove the UTC offset in seconds.  This gives local time
            tm weatherTm = *localtime(&weatherTime); 

            text =
                uiText(TextId::LastUpdate) + timeToString(weatherTm, morning) +
                " " + dateToString(weatherTm); 
            putAmPmIndicators(frame, morning);

            break;
        }
        case Sunset:
        {
            time_t weatherTime = info.sunSet + info.timezone;  //  Remove the UTC offset in seconds.  This gives us local time
            tm weatherTm = *localtime(&weatherTime); 

            text = uiText(TextId::Sunset) + timeToString(weatherTm, morning);
            putAmPmIndicators(frame, morning);
            break;
        }
        case Name:
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

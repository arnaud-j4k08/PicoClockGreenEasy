#include "Weather.h"
#include "Utils/Trace.h"
#include "PicoClockHw/Wifi.h"
#include "PicoClockHw/Platform.h"
#include <functional>

Weather::Weather() 
{
    #ifdef INCLUDE_WEATHER                                                    // kdkWx Is Weather configured?
        m_wx = true;                                                          // kdkWx Yes, set flag for weather processing                                
    #else                                                                     // kdkWx 
        m_wx = false;                                                         // kdkWx No, set flag to bypass weather processing
    #endif                                                                    // kdkWx       

    if (OPEN_WEATHER_MAP_URL == "" || WIFI_SSID == "" )                       // kdkWx Basic checks to see if weather URL and Wifi
        m_wx = false;                                                         // kdkWx defined.  Bypass weather processing if not.  

                                                                                      // kdkWx  Set callback for Weather calls        
    using namespace std::placeholders;                                                // kdkWx
    TRACE << "In Weather::Weather, calling setOnCompleteCallback \n";                     // kdkWx  Set Routine to be called
    m_httpReq.setOnCompleteCallback(std::bind(&Weather::onRequestComplete, this, _1));  // kdkWx  when OpenWeatherMap api 
    TRACE << "In Weather::Weather, after calling setOnCompleteCallback \n";               // kdkWx  responds
}

void Weather::syncWxNow()                                                       // kdkWx  Called from ClockUi, Weather submenu
{                                                                               // kdkWx
    startWxSync();                                                              // kdkWx  Call the OpenWeatherMap API
}                                                                               // kdkWx
                                                                                // kdkWx

void Weather::startWxSync()                                                     // kdkWx  This is also called from ClockUi 
{                                                                               // kdkWx  
    TRACE << "Weather::startWxsync";                                            // kdkWx  
    auto status = Wifi::linkStatus();                                           // kdkWx  
    TRACE << "Wifi link status: " <<Wifi::linkStatusToString(status);           // kdkWx  
    if (status != Wifi::Connected)                                              // kdkWx  
    {                                                                           // kdkWx Wifi is not connected, 
        TRACE << "Starting Wifi::connectAsync";                                 // kdkWx Start Async process to connect to Wifi
        using namespace std::placeholders;                                      // kdkWx When connected, onWifiConnectionFinished is called
        if (Wifi::connectAsync(std::bind(&Weather::onWifiConnectionFinished, this, _1)))
        {                                                                       // kdkWx
            TRACE << "Wifi::connectAsync started";                              // kdkWx
            m_wxSync = WxWaitingForWifi;                                       // kdkWx
        }                                                                       // kdkWx  
    } else                                                                      // kdkWx  
    {                                                                           // kdkWx  
        TRACE << "Already connected";                                           // kdkWx  
        if (m_wx)                                                               // kdkWx  Is Weather configured?  
        {                                                                       // kdkWx 
        TRACE << "In Weather::startWxSync, calling m_httpReq start \n";         // kdkWx  
    // Connect wi-fi if necessary (the lambda expression below will also be called if already 
    // connected)                                                               // kdkWx  Test whether we are really connected
            Wifi::connectAsync(                                                 // kdkWx
                [this](bool success)                                            // kdkWx
                {                                                               // kdkWx  
                    if (success)                                                // kdkWx  Actual call to get weather 
                        m_httpReq.start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL);  //kdkWx
                });                                                             // kdkWx 
        TRACE << "In Weather::startWxSync, after calling m_httpReq.start \n";     // kdkWx      
            m_wxSync = WxInProgress;                                           // kdkWx Update sync in progress for other processes
        }                                                                       // kdkWx  
    }                                                                           // kdkWx  
}                                                                               // kdkWx  


void Weather::onRequestComplete(const std::string &content)                       // kdkWx Call to OpenWeatherMap API is complete
{   
    std::string tempstr2;                                                    // kdkWx  used in populating WxInfo
    int tempInt = 0;                                                         // kdkWx  used in populating WxInfo
    std::string json;                                                        // kdkWx  Holds json string in populating WxInfo                                                                            // kdkWx Populate WxInfo with result
    TRACE << "In Weather::onRequestComplete: \n";
    std::cout << m_httpReq.content() <<std::endl;
    json = m_httpReq.content();                                                 // kdkWx Copy result string from receive buffer
    m_wxSync = Inactive;                                                       // kdkWx allow new synchronizations
    if (json.size() < 300)                                                      // kdkWx Not big enough, must be some sort of error
        return;

    tempstr2 = Weather::extractStr(json, "description");
    m_wxInfo.conditions = tempstr2;
    std::cout <<"conditions: " <<tempstr2  <<std::endl;
    tempstr2 = Weather::extract(json, "temp");
    m_wxInfo.ctemp = std::stof(tempstr2);
    std::cout <<"ctemp: " <<tempstr2 <<std::endl;
    tempstr2 = Weather::extract(json, "pressure");
    m_wxInfo.pressure = std::stoi(tempstr2);
    std::cout <<"pressure: " <<tempstr2 <<std::endl;
    tempstr2 = Weather::extract(json, "humidity");
    m_wxInfo.humidity = std::stoi(tempstr2);
    std::cout <<"humidity: " <<tempstr2 <<std::endl;
    tempstr2 = Weather::extract(json, "wind_speed");
    m_wxInfo.windSpeed = std::stof(tempstr2);
    std::cout <<"windSpeed: " <<tempstr2 <<std::endl;       
    tempstr2 = Weather::extract(json, "wind_deg");
    m_wxInfo.windDegree = std::stoi(tempstr2);
    tempInt = std::stoi(tempstr2);
    std::cout <<"windDegree: " <<tempstr2 <<std::endl;
    tempstr2 = "BAD";
    tempstr2 = Weather::getCardinal(tempInt);
    m_wxInfo.windCardinal = tempstr2;
    std::cout <<"windCardinal: " <<tempstr2 <<std::endl;
    // calculate and display windCardinal
    tempstr2 = Weather::extract(json, "sunrise");
    m_wxInfo.sunRise = std::stoull(tempstr2);
    std::cout <<"sunRise: " <<tempstr2 <<std::endl;
    tempstr2 = Weather::extract(json, "sunset");
    m_wxInfo.sunSet = std::stoull(tempstr2);
    std::cout <<"sunSet: " <<tempstr2 <<std::endl;
    tempstr2 = Weather::extract(json, "timezone_offset");
    m_wxInfo.wxTimezone = std::stoull(tempstr2);
    std::cout <<"wxTimeZone: " <<tempstr2 <<std::endl;
//    tempstr2 = Clock::extractStr(json, "name");
//    m_wxInfo.cityName = tempstr2;
//    std::cout <<"cityName: " <<tempstr2 <<std::endl;
    tempstr2 = Weather::extract(json, "dt");
    m_wxInfo.wxDateTime = std::stoull(tempstr2);
    std::cout <<"wxDateTime: " <<tempstr2 <<std::endl;
}

std::string Weather::extract(const std::string &json, const std::string &name)    // kdkWx Extract Integers and Floating Point
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

std::string Weather::extractStr(const std::string &json, const std::string &name)   // kdkWx Extract String variables
{
    std::string value = extract(json, name);
    return value.substr(1, value.size() - 2);
}

std::string Weather::getCardinal(int degrees) const                                 // kdkWx Derive compass direction from degree value
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

void Weather::onWifiConnectionFinished(bool success)
{
    if (success)
    {
        if (m_wx)
        {
            TRACE << "Wifi connected, start Weather request";
            startWxSync();
            TRACE << "WX Request started";
            m_wxSync = WxInProgress;
        }

        if (!m_wx)
            m_wxSync = Inactive;

    } else
    {
        TRACE << "Connection failed";
        m_wxSync = Inactive;
    }
}

void Weather::wxInfo(WxInfo &info)                              // kdkWx  Used by WeatherInfo.cpp
{                                                               // kdkWx
    info = m_wxInfo;                                            // kdkWx  m_wxInfo is a pointer to WxInfo structure in Clock.h 
}                                                               // kdkWx

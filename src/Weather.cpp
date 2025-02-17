#include "Weather.h"
#include "Utils/Trace.h"
#include "PicoClockHw/Wifi.h"
#include "PicoClockHw/Platform.h"
#include <functional>

Weather::Weather()
{
                                                                              // Set callback for Weather calls        
    using namespace std::placeholders;
    TRACE << "In Weather::Weather, calling setOnCompleteCallback \n";         // Set Routine to be called
    m_httpReq->setOnCompleteCallback(std::bind(&Weather::onRequestComplete, this, _1));  // when OpenWeatherMap api responds
    TRACE << "In Weather::Weather, after calling setOnCompleteCallback \n"; 
}

void Weather::syncWxNow()                                                      // Called from ClockUi
{                         
    startWxSync();                                                              // Call the OpenWeatherMap API
}

void Weather::startWxSync()                                                     
{                                                                              
    TRACE << "Weather::startWxsync";                                            
    auto status = Wifi::linkStatus();                                            
    TRACE << "Wifi link status: " <<Wifi::linkStatusToString(status);           
    if (status != Wifi::Connected)                                                
    {                                                                           // Wifi is not connected, 
        TRACE << "Starting Wifi::connectAsync";                                 // Start Async process to connect to Wifi
        using namespace std::placeholders;                                      // When connected, onWifiConnectionFinished is called
        if (Wifi::connectAsync(std::bind(&Weather::onWifiConnectionFinished, this, _1)))
        {
            TRACE << "Wifi::connectAsync started";                             
        } 
    } else                                                                       
    {                                                                            
        TRACE << "Already connected";                                           
        TRACE << "In Weather::startWxSync, calling m_httpReq->start \n";           
    // Connect wi-fi if necessary (the lambda expression below will also be called if already 
    // connected)                                                               // Test whether we are really connected
        Wifi::connectAsync(
            [this](bool success)
            {  
                if (success)                                                // Actual call to get weather 
                    m_httpReq->start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL);
            });
        TRACE << "In Weather::startWxSync, after calling m_httpReq->start \n";     
    }  
}  


void Weather::onRequestComplete(const std::string &content)                      // Call to OpenWeatherMap API is complete
{   
    std::string tempstr2;                                                        // Used in populating WxInfo
    int tempInt = 0;                                                             // Used in populating WxInfo
    std::string json;                                                            // Holds json string in populating WxInfo 
    TRACE << "In Weather::onRequestComplete: \n";
    TRACE << m_httpReq->content() ;
    json = m_httpReq->content();                                                 // Copy result string from receive buffer

    if (json.size() < 1)                                                         // assume an error, restart HttpRequest
    {
        TRACE << "In Weather::onRequestComplete: Resetting HttpRequest \n";
        m_httpReq.reset();
//        Wifi::deinit();
//        Wifi::init();
        m_httpReq = std::make_unique<HttpRequest>();
        using namespace std::placeholders;
        TRACE << "In Weather::onRequestComplete:, calling setOnCompleteCallback during reset of HttpRequest \n"; 
        m_httpReq->setOnCompleteCallback(std::bind(&Weather::onRequestComplete, this, _1));  
        TRACE << "In Weather::onRequestComplete:, after calling setOnCompleteCallback during reset of HttpRequest  \n"; 
        return;
    }    

    if (json.size() < 300)                                                       // Not big enough, must be some sort of error
        return;

    tempstr2 = Weather::extractStr(json, "description");
    m_wxInfo.conditions = tempstr2;
    TRACE << "conditions: " << tempstr2;
    tempstr2 = Weather::extract(json, "temp");
    m_wxInfo.ctemp = std::stof(tempstr2);
    TRACE << "ctemp: " << tempstr2;
    tempstr2 = Weather::extract(json, "pressure");
    m_wxInfo.pressure = std::stoi(tempstr2);
    TRACE << "pressure: " << tempstr2;
    tempstr2 = Weather::extract(json, "humidity");
    m_wxInfo.humidity = std::stoi(tempstr2);
    TRACE << "humidity: " << tempstr2;
    tempstr2 = Weather::extract(json, "wind_speed");
    m_wxInfo.windSpeed = std::stof(tempstr2);
    TRACE << "windSpeed: " << tempstr2;       
    tempstr2 = Weather::extract(json, "wind_deg");
    m_wxInfo.windDegree = std::stoi(tempstr2);
    tempInt = std::stoi(tempstr2);
    TRACE << "windDegree: " << tempstr2;
    tempstr2 = "BAD";
    tempstr2 = Weather::getCardinal(tempInt);
    m_wxInfo.windCardinal = tempstr2;
    TRACE << "windCardinal: " << tempstr2;
    // calculate and display windCardinal
    tempstr2 = Weather::extract(json, "sunrise");
    m_wxInfo.sunRise = std::stoull(tempstr2);
    TRACE << "sunRise: " << tempstr2;
    tempstr2 = Weather::extract(json, "sunset");
    m_wxInfo.sunSet = std::stoull(tempstr2);
    TRACE << "sunSet: " << tempstr2;
    tempstr2 = Weather::extract(json, "timezone_offset");
    m_wxInfo.wxTimezone = std::stoull(tempstr2);
    TRACE << "wxTimeZone: " << tempstr2;
//    tempstr2 = Clock::extractStr(json, "name");
//    m_wxInfo.cityName = tempstr2;
//    std::cout <<"cityName: " <<tempstr2 <<std::endl;
    tempstr2 = Weather::extract(json, "dt");
    m_wxInfo.wxDateTime = std::stoull(tempstr2);
    TRACE << "wxDateTime: " << tempstr2;
}

std::string Weather::extract(const std::string &json, const std::string &name)    // Extract Integers and Floating Point
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

std::string Weather::extractStr(const std::string &json, const std::string &name)   // Extract String variables
{
    std::string value = extract(json, name);
    return value.substr(1, value.size() - 2);
}

std::string Weather::getCardinal(int degrees) const                                 // Derive compass direction from degree value
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
        TRACE << "Wifi connected, start Weather request";
        startWxSync();
        TRACE << "WX Request started";
    } else
    {
        TRACE << "Connection failed";
    }
}

void Weather::wxInfo(WxInfo &info)                                          // Used by WeatherInfo.cpp
{                                                               
    info = m_wxInfo;                                                        // m_wxInfo is a pointer to WxInfo structure in Clock.h 
}                                                             

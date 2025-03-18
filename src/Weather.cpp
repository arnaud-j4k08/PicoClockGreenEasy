#include "Weather.h"
#include "Utils/Trace.h"
#include "PicoClockHw/Wifi.h"
#include "PicoClockHw/Platform.h"
#include <functional>

namespace
{
    // Extract Integers and Floating Point
    std::string extract(const std::string &json, const std::string &name)
    {
        std::string prefix = "\"" + name + "\":";
        auto prefixPos = json.find(prefix);
        if (prefixPos != std::string::npos)
        {
            auto beginPos = prefixPos + prefix.size();
            auto endPos = json.find(",", beginPos);
            // TODO: check endPos
            auto endPosbrace = json.find("}", beginPos);
            if (endPosbrace != std::string::npos && endPos > endPosbrace)
            { 
                endPos = endPosbrace;
            }
            return json.substr(beginPos, endPos - beginPos);
        } else
            return "";
    }

    // Extract String variables
    std::string extractStr(const std::string &json, const std::string &name) 
    {
        std::string value = extract(json, name);
        return value.substr(1, value.size() - 2);
    }

    // Derive compass direction from degree value
    std::string angleToDirection(int degrees)
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

}

Weather::Weather()
{
    // Set Routine to be called when OpenWeatherMap api responds
    TRACE << "In Weather::Weather, calling setOnCompleteCallback \n";         
    using namespace std::placeholders;
    m_httpReq->setOnCompleteCallback(std::bind(&Weather::onRequestComplete, this, _1));
    TRACE << "In Weather::Weather, after calling setOnCompleteCallback \n"; 
}

void Weather::syncWeatherNow()                                                
{                         
    startWeatherSync();
}

void Weather::startWeatherSync()                                                     
{                                                                              
    TRACE << "Weather::startWeatherSync";                                            
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
        TRACE << "In Weather::startWeatherSync, calling m_httpReq->start \n";           

        // Connect wi-fi if necessary (the lambda expression below will also be called if already 
        // connected)                                                          
        Wifi::connectAsync(
            [this](bool success)
            {  
                if (success)                                                
                {
                    TRACE <<"Starting request, URL:" <<OPEN_WEATHER_MAP_URL;
                    // TODO: find why I don't get the result
                    m_httpReq->start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL);
                }
            });
        TRACE << "In Weather::startWeatherSync, after calling m_httpReq->start \n";     
    }  
}  

// Call to OpenWeatherMap API is complete
void Weather::onRequestComplete(const std::string &content)                      
{   
    TRACE << "In Weather::onRequestComplete, content:";
    TRACE << m_httpReq->content() ;
    std::string json = m_httpReq->content(); // Copy result string from receive buffer

    // TODO: check
    if (json.size() < 300) // Not big enough, must be some sort of error
        return;

    try // TODO: catch in every conversion
    {
        std::string tempstr2 = extractStr(json, "description");
        m_weatherInfo.conditions = tempstr2;
        TRACE << "conditions: " << tempstr2;
        tempstr2 = extract(json, "temp");
        m_weatherInfo.ctemp = std::stof(tempstr2);
        TRACE << "ctemp: " << tempstr2;
        tempstr2 = extract(json, "pressure");
        m_weatherInfo.pressure = std::stoi(tempstr2);
        TRACE << "pressure: " << tempstr2;
        tempstr2 = extract(json, "humidity");
        m_weatherInfo.humidity = std::stoi(tempstr2);
        TRACE << "humidity: " << tempstr2;
        tempstr2 = extract(json, "wind_speed");
        TRACE << "wind_speed:" << tempstr2 << "(end)";
        m_weatherInfo.windSpeed = std::stof(tempstr2);
        TRACE << "windSpeed: " << tempstr2;       
        tempstr2 = extract(json, "wind_deg");
        m_weatherInfo.windDegree = std::stoi(tempstr2);
        int tempInt = std::stoi(tempstr2);
        TRACE << "windDegree: " << tempstr2;
        tempstr2 = "BAD";
        tempstr2 = angleToDirection(tempInt);
        m_weatherInfo.windCardinal = tempstr2;
        TRACE << "windCardinal: " << tempstr2;
        // calculate and display windCardinal
        tempstr2 = extract(json, "sunrise");
        m_weatherInfo.sunRise = std::stoull(tempstr2);
        TRACE << "sunRise: " << tempstr2;
        tempstr2 = extract(json, "sunset");
        m_weatherInfo.sunSet = std::stoull(tempstr2);
        TRACE << "sunSet: " << tempstr2;
        tempstr2 = extract(json, "timezone_offset");
        m_weatherInfo.timezone = std::stoull(tempstr2);
        TRACE << "timeZone: " << tempstr2;
    //    tempstr2 = Clock::extractStr(json, "name");
    //    m_weatherInfo.cityName = tempstr2;
    //    std::cout <<"cityName: " <<tempstr2 <<std::endl;
        tempstr2 = extract(json, "dt");
        m_weatherInfo.dateTime = std::stoull(tempstr2);
        TRACE << "dateTime: " << tempstr2;
   }
    catch(const std::exception& e)
    {
        TRACE << "Exception:" << e.what();
    }
    
}

void Weather::onWifiConnectionFinished(bool success)
{
    if (success)
    {
        TRACE << "Wifi connected, start Weather request";
        startWeatherSync();
        TRACE << "Weather Request started";
    } else
    {
        TRACE << "Connection failed";
    }
}

void Weather::weatherInfo(WeatherInfo &info)                                          // Used by WeatherInfo.cpp
{                                                               
    info = m_weatherInfo;                                                        // m_weatherInfo is a pointer to WeatherInfo structure in Clock.h 
}                                                             

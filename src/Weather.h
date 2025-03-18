#pragma once

#include "PicoClockHw/HttpRequest.h"            

#include <memory>
#include <time.h>

class Weather
{
public:
    Weather();

// Structure for Weather information.                   
    struct WeatherInfo                                       
    {                                                   
        std::string conditions = "No Data";             
        float ctemp = 0;                                
        int pressure = 0;                               
        int humidity = 0;                               
        float windSpeed = 0;                            
        int windDegree = 0;                             
        std::string windCardinal = "N";                 
        uint64_t sunRise = 0;                           
        uint64_t sunSet = 0;                            
        uint64_t timezone = 0;                        
        std::string cityName = "No Data"; // Not currently supported by OpenWeatherMap 3.0 API
        uint64_t dateTime = 0;                        
    };                                                  

    void syncWeatherNow(); // Intermediate stub.  Called by ClockUi
    void weatherInfo(WeatherInfo &info); // Used by WeatherInfo

private:
    void onWifiConnectionFinished(bool success);
    void startWeatherSync(); // Called during Clock initialization, and periodic updates
    void onRequestComplete(const std::string &content);

    WeatherInfo m_weatherInfo;
    std::unique_ptr<HttpRequest> m_httpReq = std::make_unique<HttpRequest>();
};
#pragma once

#include "PicoClockHw/HttpRequest.h"            

#include <memory>
#include <time.h>

class Weather
{
public:
    Weather();
// Structure for Weather information.                   
    struct WxInfo                                       
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
        uint64_t wxTimezone = 0;                        
        std::string cityName = "No Data";                                   // Not currently supported by OpenWeatherMap 3.0 API
        uint64_t wxDateTime = 0;                        
    };                                                  

    void syncWxNow();                                                       // Intermediate stub.  Called by ClockUi
    void wxInfo(WxInfo &info);                                              // Used by WeatherInfo

private:
    void onWifiConnectionFinished(bool success);
    void startWxSync();                                                     // Called during Clock initialization, and periodic updates

    WxInfo m_wxInfo;                                                        // pointer to WxInfo stucture

    time_t m_time = 0; // Current time as unix time, local (not UTC), not considering DST
    tm m_tm = {}; // Current time as tm, considering DST
#ifdef INCLUDE_WEATHER 
    HttpRequest m_httpReq;
#endif    

    void onRequestComplete(const std::string &content);                      
    std::string extract(const std::string &json, const std::string &name);   
    std::string extractStr(const std::string &json, const std::string &name); 
    std::string getCardinal(int degrees) const;                                 

    bool m_wx = true;                                                        // Boolean to test execution of Weather functions
};
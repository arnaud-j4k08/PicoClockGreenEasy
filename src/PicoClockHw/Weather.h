#pragma once

#include "PicoClockHw/HttpRequest.h"            // kdkWx

#include <memory>
#include <time.h>

class Weather
{
public:
    Weather();
// Structure for Weather information.                   //kdkWx
    struct WxInfo                                       //kdkWx
    {                                                   //kdkWx
        std::string conditions = "Zippy Dippy";             //kdkWx
        float ctemp = 50;                                //kdkWx
        int pressure = 75;                               //kdkWx
        int humidity = 0;                               //kdkWx
        float windSpeed = 10;                            //kdkWx
        int windDegree = 0;                             //kdkWx
        std::string windCardinal = "N";                 //kdkWx
        uint64_t sunRise = 0;                           //kdkWx
        uint64_t sunSet = 0;                            //kdkWx
        uint64_t wxTimezone = 0;                        //kdkWx
        std::string cityName = "No Data";               //kdkWx  Not currently supported by OpenWeatherMap 3.0 API
        uint64_t wxDateTime = 0;                        //kdkWx
    };                                                  //kdkWx

    void syncWxNow();                                               // kdkWx  Intermediate stub.  Called by ClockUi
    void wxInfo(WxInfo &info);                                      // kdkWx  Used by WeatherInfo

private:


    void onWifiConnectionFinished(bool success);
    void startWxSync();                             // kdkWx Called during Clock initialization, and periodic updates
                                                    // kdkWx Also part of Weather subMenu Action bind in ClockUi

    enum WeatherSync
    {
        Inactive,
        WxWaitingForWifi,                           // kdkWx  
        WxInProgress                                // kdkWx  
    };


    WeatherSync m_wxSync = Inactive;

    WxInfo m_wxInfo;                                                         // kdkWx  pointer to WxInfo stucture


    time_t m_time = 0; // Current time as unix time, local (not UTC), not considering DST
    tm m_tm = {}; // Current time as tm, considering DST

    HttpRequest m_httpReq;                                                   // kdkWx
    void onRequestComplete(const std::string &content);                      // kdkWx
    std::string extract(const std::string &json, const std::string &name);   // kdkWx
    std::string extractStr(const std::string &json, const std::string &name); // kdkWx
    std::string getCardinal(int degrees) const;                              // kdkWx   

    bool m_wx = true;                                                        // kdkWx  Boolean to test execution of Weather functions
};
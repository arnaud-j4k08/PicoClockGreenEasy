#include "Weather.h"
#include "Utils/Trace.h"
#include "PicoClockHw/Wifi.h"
#include <functional>

Weather::Weather()
{
    using namespace std::placeholders;
    m_httpReq.setOnCompleteCallback(std::bind(&Weather::onRequestComplete, this, _1));
}

void Weather::sync()
{
    // Connect wi-fi if necessary (the lambda expression below will also be called if already 
    // connected)
    Wifi::connectAsync(
        [this](bool success)
        {
            if (success) 
                m_httpReq.start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL); 
        });
}

void Weather::onRequestComplete(const std::string &content)
{
    TRACE << "Result of HTTP request:";
    std::cout << m_httpReq.content() <<std::endl;
}
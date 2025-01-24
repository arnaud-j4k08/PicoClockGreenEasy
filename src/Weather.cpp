#include "Weather.h"
#include "Utils/Trace.h"
#include <functional>

Weather::Weather()
{
    using namespace std::placeholders;
    m_httpReq.setOnCompleteCallback(std::bind(&Weather::onRequestComplete, this, _1));
}

void Weather::sync()
{
    // TODO: connect to wifi if needed
    m_httpReq.start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL);
}

void Weather::onRequestComplete(const std::string &content)
{
    TRACE << "Result of HTTP request:";
    std::cout << m_httpReq.content() <<std::endl;
}
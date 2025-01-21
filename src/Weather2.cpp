#include "Weather2.h"
#include "Utils/Trace.h"
#include <functional>
Weather2::Weather2()
{
    TRACE << "In Weather2 Weather2 \n";
    using namespace std::placeholders;
    m_httpReq.setOnCompleteCallback(std::bind(&Weather2::onRequestComplete, this, _1));
}
void Weather2::sync()
{
    TRACE << "In Weather2 sync \n";
    m_httpReq.start("api.openweathermap.org", 443, OPEN_WEATHER_MAP_URL);
}
void Weather2::onRequestComplete(const std::string &content)
{
    TRACE << "In Weather2 onRequestComplete \n";
    TRACE << "Result of HTTP request:";
    std::cout << m_httpReq.content() <<std::endl;
}
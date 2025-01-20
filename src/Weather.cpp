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
    m_httpReq.start("www.google.com", 443, "/");
}

void Weather::onRequestComplete(const std::string &content)
{
    TRACE << "Result of HTTP request:";
    std::cout << m_httpReq.content() <<std::endl;
}
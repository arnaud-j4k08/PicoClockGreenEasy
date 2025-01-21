#pragma once
#include "PicoClockHw/HttpRequest.h"
#include <string>
class Weather2
{
public:
    Weather2();
    void sync();
private:
    HttpRequest m_httpReq;
    void onRequestComplete(const std::string &content);
};
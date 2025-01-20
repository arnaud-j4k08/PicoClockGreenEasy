#pragma once

#include "PicoClockHw/HttpRequest.h"
#include <string>

class Weather
{
public:
    Weather();
    void sync();

private:
    HttpRequest m_httpReq;

    void onRequestComplete(const std::string &content);
};
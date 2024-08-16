#include "Trace.h"
#include "PicoClockHw/Platform.h"

#include <time.h>

namespace {
uint64_t g_lastTrace = 0;

bool isEnabledForFile(const std::string &file)
{
    return 
        file.find("Clock.cpp") != std::string::npos ||
        file.find("main.cpp") != std::string::npos ||
        file.find("Rtc.cpp") != std::string::npos;
}
} // namespace

Trace::Trace(const char *file, int line) : m_enabled(isEnabledForFile(file))
{
    if (!m_enabled)
        return;
    
    std::cout << file << ":" << line << " ";

    // For performance tuning, also print the elapsed microseconds since the last trace
    uint64_t now = Platform::timeUs();
    std::cout << "+" << now - g_lastTrace << " ";
    g_lastTrace = now;
}

Trace::~Trace()
{
    if (m_enabled)
        std::cout << "\n";
}

Trace &Trace::operator <<(const SetAutoSpace &sas)
{
    m_autoSpace = sas.autoSpace();
    return *this;
}

Trace &Trace::operator <<(const tm &dateTime)
{
    // (the asctime function would have been convenient for that but calling it causes program termination.)
    *this   
        << SetAutoSpace(false)
        << dateTime.tm_mday << "/" << dateTime.tm_mon + 1 << "/" << dateTime.tm_year + 1900 << " "
        << " wday: " <<dateTime.tm_wday <<" "
        << dateTime.tm_hour << ":"  <<dateTime.tm_min << ":" << dateTime.tm_sec;
    return *this;
}

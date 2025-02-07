#include "Trace.h"
#include "PicoClockHw/Platform.h"

#include <time.h>
#include <string.h>

namespace {
uint64_t g_lastTrace = 0;

bool isEnabledForFile(const std::string &file)
{
    return 
//        file == "main.cpp" ||
//        file == "HttpRequest.cpp" ||
//        file == "Weather.cpp" ||
        file == "Gps.cpp" ||
        file == "ClockUi.cpp";
}
} // namespace

Trace::Trace(const char *filePath, int line) : m_enabled()
{
    // Remove the path before the file name to make output shorter.
    const char *file = strrchr(filePath, '/');
    if (file == nullptr)
        file = filePath;
    else
        file++;

    m_enabled = isEnabledForFile(file);

    if (!m_enabled)
        return;
    
    std::cout << file << ":" <<std::dec << line << " ";

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
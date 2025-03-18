#pragma once

#include <pico/time.h>
#include <string>
#include <functional>
#include <time.h>

class Gps
{
public:
    Gps();
    ~Gps();
    void setEnabled(bool enabled);
    void setTimeCallback(std::function<void(time_t utcTime, uint32_t ms)> c)
    {
        m_timeCallback = c;
    }
    void setTimeoutCallback(std::function<void()> c)
    {
        m_timeoutCallback = c;
    }

private:
    static void onUartRx();
    void onMessage(const std::string &msg);
    void onDateTime(const std::string &date, const std::string &time);
    void resetTimeoutAlarm();
    int64_t onTimeout(alarm_id_t);

    static Gps *m_instance;
    std::string g_receiveBuffer;
    std::function<void(time_t utcTime, uint32_t ms)> m_timeCallback;
    std::function<void()> m_timeoutCallback;
    alarm_id_t m_timeoutAlarm = -1;
};
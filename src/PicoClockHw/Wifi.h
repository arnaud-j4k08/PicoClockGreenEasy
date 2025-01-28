#pragma once

#include <string>
#include <list>
#include <functional>
#include <pico/time.h>

class Wifi
{
public:
    enum Status
    {
        Unknown,
        OK,
        NotAvailable,
        Down,
        Connecting,
        NoIp,
        Connected,
        ConnectionFailed,
        NoNetworkFound,
        AuthenticationFailed,
    };

    static bool init();
    static void deinit();
    static bool connectBlocking();
    
    // Can be called also if a connection is already ongoing. In this case, all passed callbacks
    // will be called when the connection succeeds or fails.
    // TODO: test connection failure
    static bool connectAsync(const std::function<void(bool success)> &finishedCallback);
    
    static Status linkStatus();
    static std::string linkStatusToString(Status s);

private:
    static int64_t monitorConnection(alarm_id_t id, void *user_data);
    static bool handleConnectResult(int res);

    static Status m_connectResult;
    static alarm_id_t m_monitorConnectionAlarm;
    static std::list<std::function<void(bool success)>> m_connectionFinishedCallbacks;
};

#ifndef PICO_CYW43_SUPPORTED
inline bool Wifi::init()
{
    return false;
}

inline void Wifi::deinit()
{}

inline bool Wifi::connectBlocking()
{
    return false;
}

inline bool Wifi::connectAsync()
{
    return false;
}

inline Wifi::Status Wifi::linkStatus()
{
    return NotAvailable;
}

inline std::string Wifi::linkStatusToString(Status s)
{
    return "";
}

#endif

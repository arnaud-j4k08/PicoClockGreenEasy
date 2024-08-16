#pragma once

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
    static Status linkStatus();

private:
    static Status m_connectResult;
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

inline Wifi::Status Wifi::linkStatus()
{
    return NotAvailable;
}
#endif

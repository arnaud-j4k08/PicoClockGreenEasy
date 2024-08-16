#pragma once

#ifdef PICO_CYW43_SUPPORTED
#include <lwip/dns.h>
#endif

#include <pico/stdlib.h>
#include <functional>
#include <time.h>

class Ntp
{
public:
    enum State
    {
        Idle,
        WaitingForDns,
        WaitingForResponse,
        Done,
        DnsFailed,
        InvalidResponse,
        Timeout
    };

    bool init();
#ifdef PICO_CYW43_SUPPORTED
    ~Ntp();
#endif
    void setTimeCallback(std::function<void(time_t utcTime, uint32_t ms)> c)
    {
#ifdef PICO_CYW43_SUPPORTED
        m_timeCallback = c;
#endif
    }
    void setFailCallback(std::function<void(State reason)> c)
    {
#ifdef PICO_CYW43_SUPPORTED
        m_failCallback = c;
#endif
    }
    void startRequest();
    State state() const
    {
        return m_state;
    }

private:
#ifdef PICO_CYW43_SUPPORTED
    int64_t onNtpFailed(alarm_id_t id);
    void onNtpDnsFound(const char *hostname, const ip_addr_t *ipaddr);
    void sendNtpRequest();
    void onMsgReceived(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

    ip_addr_t m_serverAddress;
    udp_pcb *m_pcb = nullptr; // protocol control block
    alarm_id_t m_timeoutAlarm = -1;
    std::function<void(time_t utcTime, uint32_t ms)> m_timeCallback;
    std::function<void(State reason)> m_failCallback;
#endif

    State m_state = Idle;
};

#ifndef PICO_CYW43_SUPPORTED
inline bool Ntp::init() 
{
    return false;
}

inline void Ntp::startRequest()
{
}
#endif
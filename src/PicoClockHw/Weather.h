// This module is very much a work in progress.   Trying to cram the working picow_http_client code into a version of Ntp.h
// Putting it out there so that Arnaud can look at it, and comment   kdkWx

#pragma once


#include <pico/stdlib.h>
#include <functional>
#include <time.h>

#include <string>

#include "lwip/altcp_tls.h"
#include "lwip/apps/http_client.h"
#include "Functions/AbstractFunction.h"

class Weather 
//class Weather : public AbstractFunction
{
public:

    //HttpRequest();
    //Weather();
    void startRequest();
    bool isComplete() const;
    std::string content() const;
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
    ~Weather();    // kdkWx  was ~Ntp
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
//    void startRequest();
    State state() const
    {
        return m_state;
    }

private:

    altcp_pcb *tlsAlloc(u8_t ip_type);
    err_t receive(struct altcp_pcb *conn, struct pbuf *p, err_t err);
    void onTransferComplete(
        httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err);
    std::string extract(const std::string &json, const std::string &name);
    std::string extractStr(const std::string &json, const std::string &name);    
    altcp_tls_config *m_tlsConfig = nullptr;
    httpc_connection_t m_settings = {};
    altcp_allocator_t m_tlsAllocator = {};
    
    std::string m_content;
    std::string json;
    std::string tempstr2;

    bool m_complete = false;

#ifdef PICO_CYW43_SUPPORTED
//    int64_t onWxFailed(alarm_id_t id);
//    void onWxDnsFound(const char *hostname, const ip_addr_t *ipaddr);
//    void sendWxRequest();
//    void onMsgReceived(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
    std::string getCardinal(int degrees) const;

    ip_addr_t m_serverAddress;
//  udp_pcb *m_pcb = nullptr; // protocol control block
    alarm_id_t m_timeoutAlarm = -1;
    std::function<void(time_t utcTime, uint32_t ms)> m_timeCallback;
    std::function<void(State reason)> m_failCallback;
#endif
    int tempInt = 0;
    State m_state = Idle;
};

#ifndef PICO_CYW43_SUPPORTED
inline bool Weather::init() 
{
    return false;
}

inline void Weather::startRequest()
{
}
#endif
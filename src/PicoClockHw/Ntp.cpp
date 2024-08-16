#include "Ntp.h"
#include "Utils/Trampoline.h"
#include "Utils/Trace.h"

#include <string.h>
#include <time.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <pico/cyw43_arch.h>

namespace
{
    const char *NTP_SERVER = "pool.ntp.org";
    unsigned int NTP_PORT = 123;
    unsigned int NTP_MSG_LEN = 48;
    unsigned NTP_TIMEOUT_MS = 10 * 1000;

    uint32_t fromBigEndian(const uint8_t buf[4])
    {
        return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    }
}

bool Ntp::init() 
{
    m_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (m_pcb == nullptr) 
    {
        TRACE <<"failed to create PCB";
        return false;
    }

    // Install callback for receiving UDP messages from the NTP server
    MAKE_TRAMPOLINE(Ntp, onMsgReceived, userPtrAtBegin);
    udp_recv(m_pcb, onMsgReceived, this);

    return true;
}

Ntp::~Ntp()
{
    udp_remove(m_pcb);
}

void Ntp::startRequest()
{
    // Set alarm in case udp requests are lost
    MAKE_TRAMPOLINE(Ntp, onNtpFailed, userPtrAtEnd);
    m_timeoutAlarm = add_alarm_in_ms(NTP_TIMEOUT_MS, onNtpFailed, this, true);

    // Get server address from DNS
    // Note: cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    cyw43_arch_lwip_begin();
    MAKE_TRAMPOLINE(Ntp, onNtpDnsFound, userPtrAtEnd);
    int err = dns_gethostbyname(NTP_SERVER, &m_serverAddress, onNtpDnsFound, this);
    cyw43_arch_lwip_end();

    switch (err)
    {
        case ERR_OK:
            sendNtpRequest(); // DNS result was cached, proceed with the NTP request
            break;
        case ERR_INPROGRESS:
            m_state = WaitingForDns;
            break;
        default:
            TRACE <<"dns request failed";
            m_state = DnsFailed;

            if (m_failCallback)
                m_failCallback(DnsFailed);
    }
}

// Callback for dns_gethostbyname
void Ntp::onNtpDnsFound(const char *hostname, const ip_addr_t *ipaddr) 
{
    if (ipaddr != nullptr) 
    {
        m_serverAddress = *ipaddr;
        TRACE <<"ntp address:"<<ipaddr_ntoa(ipaddr);
        sendNtpRequest();
    } else 
    {
        TRACE <<"ntp dns request failed";
        m_state = DnsFailed;
        if (m_failCallback)
            m_failCallback(DnsFailed);
    }
}

void Ntp::sendNtpRequest() 
{
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    auto req = static_cast<uint8_t *>(p->payload);
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b;
    udp_sendto(m_pcb, p, &m_serverAddress, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
    m_state = WaitingForResponse;
}

// Callback for udp_recv on NTP data reception
void Ntp::onMsgReceived(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) 
{
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &m_serverAddress) && 
        port == NTP_PORT && 
        p->tot_len == NTP_MSG_LEN &&
        mode == 0x4 && 
        stratum != 0) 
    {
        uint8_t timestampBuf[8] = {0};
        pbuf_copy_partial(p, timestampBuf, sizeof(timestampBuf), 40);
        uint32_t secondsSince1900 = fromBigEndian(timestampBuf);
        uint32_t ms = fromBigEndian(timestampBuf + 4) / 4294967;
        
        // Substract the number of seconds between 1 Jan 1900 and 1 Jan 1970.
        time_t secondsSince1970 = secondsSince1900 - 2208988800;
        
        m_state = Done;

        if (m_timeCallback)
            m_timeCallback(secondsSince1970, ms);
    } 
    else 
    {
        TRACE <<"invalid ntp response";
        m_state = InvalidResponse;
    
        if (m_failCallback)
            m_failCallback(InvalidResponse);
    }
    pbuf_free(p);

    if (m_timeoutAlarm != -1) 
    {
        cancel_alarm(m_timeoutAlarm);
        m_timeoutAlarm = -1;
    }
}

// Callback for add_alarm_in_ms
int64_t Ntp::onNtpFailed(alarm_id_t id)
{
    m_timeoutAlarm = -1;
    TRACE <<"ntp request failed";
    m_state = Timeout;
    
    if (m_failCallback)
        m_failCallback(Timeout);
    
    return 0;
}

// This module is very much a work in progress.   Trying to cram the working picow_http_client code into a version of Ntp.cpp
// Putting it out there so that Arnaud can look at it, and comment   kdkWx

#include "Weather.h"
#include "Utils/Trampoline.h"
#include "Utils/Trace.h"

#include <string.h>
#include <time.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <pico/cyw43_arch.h>      // also in client example  


#include <iostream>             // from Arnaud's email for json parsing

namespace
{
    const char *WX_SERVER = "pool.ntp.org";
    const char *Weather_HOST = "api.openweathermap.org";
    const char *Weather_URL_Request = "/data/2.5/weather?zip=20141,us&appid=SuperSecretKey&units=imperial";
    unsigned int WX_PORT = 123;
    unsigned int WX_MSG_LEN = 48;
    unsigned WX_TIMEOUT_MS = 10 * 1000;

    uint32_t fromBigEndian(const uint8_t buf[4])
    {
        return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    }
}

bool Weather::init() 
{
    m_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (m_pcb == nullptr) 
    {
        TRACE <<"failed to create PCB";
        return false;
    }

    // Install callback for receiving UDP messages from the NTP server
    MAKE_TRAMPOLINE(Weather, onMsgReceived, userPtrAtBegin);
    udp_recv(m_pcb, onMsgReceived, this);

    return true;
}

Weather::~Weather()
{
    udp_remove(m_pcb);
}

void Weather::startRequest()
{
    // Set alarm in case http requests are lost
    MAKE_TRAMPOLINE(Weather, onWxFailed, userPtrAtEnd);
    m_timeoutAlarm = add_alarm_in_ms(WX_TIMEOUT_MS, onWxFailed, this, true);

    sendWxRequest(); // DNS result was cached, proceed with the NTP request
  
}


void Weather::sendWxRequest() 
{

    m_state = WaitingForResponse;

//    EXAMPLE_HTTP_REQUEST_T req3 = {0};
//    req3.hostname = "api.openweathermap.org";
//    req3.url = "/data/2.5/weather?zip=20141,us&appid=SuperSecretKey&units=imperial";
//    req3.headers_fn = http_client_header_print_fn;
//    req3.recv_fn = http_client_receive_print_fn;
// Problems start here
//    req3.tls_config = altcp_tls_create_config_client(NULL, 0); // https
printf("Beginning my Weather HTTPs Request\n");    
//    int result = http_client_request_sync(cyw43_arch_async_context(), &req3);
printf("\nResult of my Weather HTTPs Request:  ");

}

void Weather::onMsgReceived(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) 
{

//  Potential Command to save buffer to json string
// json += std::string((char *)p->payload, p->len);

    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &m_serverAddress) && 
        port == WX_PORT && 
        p->tot_len == WX_MSG_LEN &&
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
int64_t Weather::onWxFailed(alarm_id_t id)
{
    m_timeoutAlarm = -1;
    TRACE <<"Wx request failed";
    m_state = Timeout;
    
    if (m_failCallback)
        m_failCallback(Timeout);
    
    return 0;
}

//  The following is from Arnaud's email

std::string extract(const std::string &json, const std::string &name)
{
    std::string prefix = "\"" + name + "\":";
    auto prefixPos = json.find(prefix);
    if (prefixPos != std::string::npos)
    {
        auto beginPos = prefixPos + prefix.size();
        auto endPos = json.find(",", beginPos);
        return json.substr(beginPos, endPos - beginPos);
    } else
        return "";
}

std::string extractStr(const std::string &json, const std::string &name)
{
    std::string value = extract(json, name);
    return value.substr(1, value.size() - 2);
}

int ExtractDataFromJson()   // extract the weather fields from the received json string. 
{
// json += std::string((char *)p->payload, p->len);  This will be in the onMessageReceived    
    std::string json = R"({"coord":{"lon":-77.7802,"lat":39.1164},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01d"}],"base":"stations","main":{"temp":19.18,"feels_like":6.94,"temp_min":15.21,"temp_max":22.23,"pressure":1022,"humidity":61,"sea_level":1022,"grnd_level":999},"visibility":10000,"wind":{"speed":11.5,"deg":320,"gust":16.11},"clouds":{"all":0},"dt":1736517544,"sys":{"type":2,"id":2003615,"country":"US","sunrise":1736512199,"sunset":1736546826},"timezone":-18000,"id":0,"name":"Round Hill","cod":200})";

    std::cout <<"conditions: " <<extractStr(json, "description") <<std::endl;
    std::cout <<"ctemp: " <<extract(json, "temp") <<std::endl;
    std::cout <<"pressure: " <<extract(json, "pressure") <<std::endl;
    std::cout <<"humidity: " <<extract(json, "humidity") <<std::endl;        
    std::cout <<"windSpeed: " <<extract(json, "speed") <<std::endl;
    std::cout <<"windDegree: " <<extract(json, "deg") <<std::endl;
    // calculate and display windCardinal
    std::cout <<"sunRise: " <<extract(json, "sunrise") <<std::endl;
    std::cout <<"sunSet: " <<extract(json, "sunset") <<std::endl;
    std::cout <<"wxTimeZone: " <<extract(json, "timezone") <<std::endl;
    std::cout <<"cityName: " <<extractStr(json, "name") <<std::endl;
    std::cout <<"wxDateTime: " <<extract(json, "dt") <<std::endl;
// TRACE <<"Temperature:" <<extract(json, "temp") <<std::endl;

//    m_wxInfo.conditions = info.conditions;                    // kdkWx
//    m_wxInfo.ctemp = info.ctemp;                              // kdkWx
//    m_wxInfo.pressure = info.pressure;                        // kdkWx
//    m_wxInfo.humidity = info.humidity;                        // kdkWx
//    m_wxInfo.windSpeed = info.WindSpeed;                      // kdkWx
//    m_wxInfo.windDegree = info.windDegree;                    // kdkWx
//    m_wxInfo.windCardinal = info.Cardinal;                    // kdkWx
//    m_wxInfo.sunRise = info.sunRise;                          // kdkWx
//    m_wxInfo.sunSet = info.sunSet;                            // kdkWx
//    m_wxInfo.wxTimezone = info.wxTimezone;                    // kdkWx
//    m_wxInfo.cityName = info.cityName;                        // kdkWx
//    m_wxInfo.wxDateTime = info.DateTime;                      // kdkWx           

    return 0;
}

//std::string Weather::getCardinal(int degrees) const   // This is currently a lot to do each second.  If it is done on sync, not so much kdkWx
//{
//    if (degrees < 22) return "N";
//    if (degrees < 67) return "NE";
//    if (degrees < 113) return "E";
//    if (degrees < 158) return "SE";
//    if (degrees < 202) return "S";
//    if (degrees < 248) return "SW";
//    if (degrees < 293) return "W";
//    if (degrees < 338) return "NW";
//    return "N";

//    if (degrees < 11) return "N";
//    if (degrees < 34) return "NNE";
//    if (degrees < 56) return "NE";
//    if (degrees < 79) return "ENE";
//    if (degrees < 101) return "E";
//    if (degrees < 123) return "ESE";
//    if (degrees < 146) return "SE";
//    if (degrees < 169) return "SSE";
//    if (degrees < 191) return "S";
//    if (degrees < 214) return "SSW";
//    if (degrees < 236) return "SW";
//    if (degrees < 259) return "WSW";
//    if (degrees < 282) return "W";
//    if (degrees < 304) return "WNW";
//    if (degrees < 327) return "NW";
//    if (degrees < 349) return "NNW";
//    return "N";

//}
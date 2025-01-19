#include "HttpRequest.h"
#include "Utils/Trace.h"
#include "Utils/Trampoline.h"
#include "Clock.h"



HttpRequest::HttpRequest()
{
    m_tlsConfig = altcp_tls_create_config_client(NULL, 0);
    MAKE_TRAMPOLINE(HttpRequest, tlsAlloc, userPtrAtBegin);
    m_tlsAllocator.alloc = tlsAlloc;
    m_tlsAllocator.arg = this;

    // TODO: add a destructor that releases m_tlsConfig
}
void HttpRequest::start()
{
    TRACE << "We made it to HttpRequest::Start\n ";
    m_content.clear();
    m_complete = false;
    MAKE_TRAMPOLINE(HttpRequest, onTransferComplete, userPtrAtBegin);
    m_settings.result_fn = onTransferComplete;
    m_settings.altcp_allocator = &m_tlsAllocator;
    MAKE_TRAMPOLINE(HttpRequest, receive, userPtrAtBegin);
    httpc_get_file_dns(
        "api.openweathermap.org",  // server_name,
        443,       // port,
        OPEN_WEATHER_MAP_URL,
//        "/data/2.5/weather?zip=20141,us&appid=SuperSecretKey&units=imperial",
        &m_settings,  // settings,
        receive,  // altcp_recv_fn recv_fn,
        this,  // callback_arg,
        nullptr); // connection)
}
bool HttpRequest::isComplete() const
{
    //std::cout << "In HttpRequest::isComplete \n";
    return m_complete;
}
std::string HttpRequest::content() const
{
    TRACE << "In HttpRequest::content\n";
    return m_content;
}
altcp_pcb *HttpRequest::tlsAlloc(u8_t ip_type)
{
    TRACE << "In HttpRequest::tlsAlloc\n";
    return altcp_tls_alloc(m_tlsConfig, ip_type);
}
void HttpRequest::onTransferComplete(
    httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    TRACE << "In HttpRequest::onTransferComplete\n";
    m_complete = true;
}
err_t HttpRequest::receive(struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    TRACE << "In HttpRequest::receive\n";
    TRACE << "Received" <<p->len <<"bytes";
    m_content += std::string(static_cast<char *>(p->payload), p->len);
    json += std::string(static_cast<char *>(p->payload), p->len);
  
    if (p->len < 400)
        return ERR_OK;

    Clock::WxInfo info2;
    //clock().wxInfo(info2);

    tempstr2 = HttpRequest::extractStr(json, "description");
    info2.conditions = tempstr2;
    std::cout <<"conditions: " <<tempstr2  <<std::endl;
//    std::cout <<"conditions: " <<HttpRequest::extractStr(json, "description") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "temp");
    info2.ctemp = std::stof(tempstr2);
    std::cout <<"ctemp: " <<tempstr2 <<std::endl;
//    std::cout <<"ctemp: " <<HttpRequest::extract(json, "temp") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "pressure");
    info2.pressure = std::stoi(tempstr2);
    std::cout <<"pressure: " <<tempstr2 <<std::endl;
//    std::cout <<"pressure: " <<HttpRequest::extract(json, "pressure") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "humidity");
    info2.humidity = std::stoi(tempstr2);
    std::cout <<"humidity: " <<tempstr2 <<std::endl;
//    std::cout <<"humidity: " <<HttpRequest::extract(json, "humidity") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "speed");
    info2.windSpeed = std::stof(tempstr2);
    std::cout <<"windSpeed: " <<tempstr2 <<std::endl;       
//    std::cout <<"windSpeed: " <<HttpRequest::extract(json, "speed") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "deg");
    info2.windDegree = std::stoi(tempstr2);
    std::cout <<"windDegree: " <<tempstr2 <<std::endl;
//    std::cout <<"windDegree: " <<HttpRequest::extract(json, "deg") <<std::endl;
    info2.windCardinal = "BAD";
    // calculate and display windCardinal
    tempstr2 = HttpRequest::extract(json, "sunrise");
    info2.sunRise = std::stoull(tempstr2);
    std::cout <<"sunRise: " <<tempstr2 <<std::endl;
//    std::cout <<"sunRise: " <<HttpRequest::extract(json, "sunrise") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "sunset");
    info2.sunSet = std::stoull(tempstr2);
    std::cout <<"sunSet: " <<tempstr2 <<std::endl;
//    std::cout <<"sunSet: " <<HttpRequest::extract(json, "sunset") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "timezone");
    info2.wxTimezone = std::stoull(tempstr2);
    std::cout <<"wxTimeZone: " <<tempstr2 <<std::endl;
//    std::cout <<"wxTimeZone: " <<HttpRequest::extract(json, "timezone") <<std::endl;
    tempstr2 = HttpRequest::extractStr(json, "name");
    info2.cityName = tempstr2;
    std::cout <<"cityName: " <<tempstr2 <<std::endl;
//    std::cout <<"cityName: " <<HttpRequest::extractStr(json, "name") <<std::endl;
    tempstr2 = HttpRequest::extract(json, "dt");
    info2.wxDateTime = std::stoull(tempstr2);
    std::cout <<"wxDateTime: " <<tempstr2 <<std::endl;
//    std::cout <<"wxDateTime: " <<HttpRequest::extract(json, "dt") <<std::endl;

    //clock().logWeather(info2);

    return ERR_OK;
  
}
//  The following is from Arnaud's email

std::string HttpRequest::extract(const std::string &json, const std::string &name)
{
    std::string prefix = "\"" + name + "\":";
    auto prefixPos = json.find(prefix);
    if (prefixPos != std::string::npos)
    {
        auto beginPos = prefixPos + prefix.size();
        auto endPos = json.find(",", beginPos);
        auto endPosbrace = json.find("}", beginPos);
        if (endPosbrace != std::string::npos && endPos > endPosbrace)
        { 
            endPos = endPosbrace;
        }
        return json.substr(beginPos, endPos - beginPos);
    } else
        return "";
}

std::string HttpRequest::extractStr(const std::string &json, const std::string &name)
{
    std::string value = extract(json, name);
    return value.substr(1, value.size() - 2);
}
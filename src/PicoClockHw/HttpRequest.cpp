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
        "/data/2.5/weather?zip=20141,us&appid=SuperSecretKey&units=imperial",       // const char *uri,
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
  
    std::cout <<"conditions: " <<HttpRequest::extractStr(json, "description") <<std::endl;
    std::cout <<"ctemp: " <<HttpRequest::extract(json, "temp") <<std::endl;
    std::cout <<"pressure: " <<HttpRequest::extract(json, "pressure") <<std::endl;
    std::cout <<"humidity: " <<HttpRequest::extract(json, "humidity") <<std::endl;        
    std::cout <<"windSpeed: " <<HttpRequest::extract(json, "speed") <<std::endl;
    std::cout <<"windDegree: " <<HttpRequest::extract(json, "deg") <<std::endl;
    // calculate and display windCardinal
    std::cout <<"sunRise: " <<HttpRequest::extract(json, "sunrise") <<std::endl;
    std::cout <<"sunSet: " <<HttpRequest::extract(json, "sunset") <<std::endl;
    std::cout <<"wxTimeZone: " <<HttpRequest::extract(json, "timezone") <<std::endl;
    std::cout <<"cityName: " <<HttpRequest::extractStr(json, "name") <<std::endl;
    std::cout <<"wxDateTime: " <<HttpRequest::extract(json, "dt") <<std::endl;

    Clock::WxInfo info2;
    //clock().wxInfo(info2);
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
        return json.substr(beginPos, endPos - beginPos);
    } else
        return "";
}

std::string HttpRequest::extractStr(const std::string &json, const std::string &name)
{
    std::string value = extract(json, name);
    return value.substr(1, value.size() - 2);
}
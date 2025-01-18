#pragma once

#include <string>

#include "lwip/altcp_tls.h"
#include "lwip/apps/http_client.h"
#include "Functions/AbstractFunction.h"

class HttpRequest  
{
public:
    HttpRequest();
    void start();
    bool isComplete() const;
    std::string content() const;

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
};
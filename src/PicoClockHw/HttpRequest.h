#pragma once

#include <string>
#include <functional>

#include "lwip/altcp_tls.h"
#include "lwip/apps/http_client.h"

class HttpRequest
{
public:
    HttpRequest();
    void setOnCompleteCallback(std::function<void(std::string)> callback);
    void start(const std::string &serverName, uint16_t port, const std::string &uri);
    bool isComplete() const;
    std::string content() const;

private:
    altcp_pcb *tlsAlloc(u8_t ip_type);
    err_t receive(struct altcp_pcb *conn, struct pbuf *p, err_t err);
    void onTransferComplete(
        httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err);

    altcp_tls_config *m_tlsConfig = nullptr;
    httpc_connection_t m_settings = {};
    altcp_allocator_t m_tlsAllocator = {};

    std::function<void(std::string)> m_onCompleteCallback;
    std::string m_content;
    bool m_complete = false;
};
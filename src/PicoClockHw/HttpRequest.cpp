#include "HttpRequest.h"

#include "Utils/Trace.h"
#include "Utils/Trampoline.h"

HttpRequest::HttpRequest()
{
    TRACE << "altcp_tls_create_config_client";
    m_tlsConfig = altcp_tls_create_config_client(NULL, 0);
    TRACE << "done";

    MAKE_TRAMPOLINE(HttpRequest, tlsAlloc, userPtrAtBegin);
    m_tlsAllocator.alloc = tlsAlloc;
    m_tlsAllocator.arg = this;

    // TODO: add a destructor that releases m_tlsConfig
}

void HttpRequest::setOnCompleteCallback(std::function<void(std::string)> callback)
{
    m_onCompleteCallback = callback;
}

void HttpRequest::start(const std::string &serverName, uint16_t port, const std::string &uri)
{
    m_content.clear();
    m_complete = false;

    MAKE_TRAMPOLINE(HttpRequest, onTransferComplete, userPtrAtBegin);
    m_settings.result_fn = onTransferComplete;

    m_settings.altcp_allocator = &m_tlsAllocator;

    MAKE_TRAMPOLINE(HttpRequest, receive, userPtrAtBegin);
    httpc_get_file_dns(
        serverName.c_str(),
        port,
        uri.c_str(),
        &m_settings,
        receive,    // altcp_recv_fn recv_fn,
        this,       // callback_arg,
        nullptr);   // connection)
}

bool HttpRequest::isComplete() const
{
    return m_complete;
}

std::string HttpRequest::content() const
{
    return m_content;
}

altcp_pcb *HttpRequest::tlsAlloc(u8_t ip_type)
{
    return altcp_tls_alloc(m_tlsConfig, ip_type);
}

err_t HttpRequest::receive(struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    TRACE << "Received" <<p->len <<"bytes";
    m_content += std::string(static_cast<char *>(p->payload), p->len);
    return ERR_OK;
}

void HttpRequest::onTransferComplete(
    httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    // TODO: check why this function is called only 14 seconds after the last received block of 
    // bytes

    TRACE << "onTransferComplete";
    m_complete = true;

    if (m_onCompleteCallback)
        m_onCompleteCallback(m_content);
}


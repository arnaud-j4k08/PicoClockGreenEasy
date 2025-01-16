#include "HttpRequest.h"

#include "Utils/Trace.h"
#include "Utils/Trampoline.h"

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
    m_content.clear();
    m_complete = false;

    MAKE_TRAMPOLINE(HttpRequest, onTransferComplete, userPtrAtBegin);
    m_settings.result_fn = onTransferComplete;

    m_settings.altcp_allocator = &m_tlsAllocator;

    MAKE_TRAMPOLINE(HttpRequest, receive, userPtrAtBegin);
    httpc_get_file_dns(
        "www.google.com",  // server_name,
        443,       // port,
        "/",       // const char *uri,
        &m_settings,  // settings,
        receive,  // altcp_recv_fn recv_fn,
        this,  // callback_arg,
        nullptr); // connection)
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

void HttpRequest::onTransferComplete(
    httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    m_complete = true;
}

err_t HttpRequest::receive(struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    TRACE << "Received" <<p->len <<"bytes";
    m_content += std::string(static_cast<char *>(p->payload), p->len);
    return ERR_OK;
}
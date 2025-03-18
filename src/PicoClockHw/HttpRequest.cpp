#include "HttpRequest.h"

#include "Utils/Trace.h"
#include "Utils/Trampoline.h"

// TODO: error handling in this whole class

HttpRequest::HttpRequest()
{
    // Prepare TLS allocator
    MAKE_TRAMPOLINE(HttpRequest, tlsAlloc, userPtrAtBegin);
    m_tlsAllocator.alloc = tlsAlloc;
    m_tlsAllocator.arg = this;

    // Prepare settings
    m_settings.headers_done_fn = onHeadersDoneProxy;
    MAKE_TRAMPOLINE(HttpRequest, onTransferComplete, userPtrAtBegin);
    m_settings.result_fn = onTransferComplete;
    m_settings.altcp_allocator = &m_tlsAllocator;

    // But it is too early to call anything in lwIP, as cyw43 may not be initialized yet.
}

HttpRequest::~HttpRequest()
{
    if (m_tlsConfig != nullptr)
        altcp_tls_free_config(m_tlsConfig);
}

void HttpRequest::setOnCompleteCallback(std::function<void(std::string)> callback)
{
    m_onCompleteCallback = callback;
}

void HttpRequest::start(const std::string &serverName, uint16_t port, const std::string &uri)
{
    // Initialize member variables to receive the result
    m_content.clear();
    m_contentLen = -1;
    m_complete = false;

    // Create the TLS config if it was not done yet
    if (m_tlsConfig == nullptr)
    {
        TRACE << "altcp_tls_create_config_client";
        m_tlsConfig = altcp_tls_create_config_client(NULL, 0);
        TRACE << "done, m_tlsConfig=" <<m_tlsConfig;
    }

    TRACE << "Start the actual request";
    MAKE_TRAMPOLINE(HttpRequest, receive, userPtrAtBegin);
    err_t e = httpc_get_file_dns(
        serverName.c_str(),
        port,
        uri.c_str(),
        &m_settings,
        receive,    // altcp_recv_fn recv_fn,
        this,       // callback_arg,
        nullptr);   // connection)
    TRACE << "httpc_get_file_dns returned" << static_cast<int>(e); 
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
    altcp_pcb *res = altcp_tls_alloc(m_tlsConfig, ip_type);
    TRACE << "altcp_tls_alloc returned" << res;
    return res;
}

err_t HttpRequest::onHeadersDoneProxy(
    httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdrLen, u32_t contentLen)
{
    return static_cast<HttpRequest *>(arg)->onHeadersDone(
        connection, hdr, hdrLen, contentLen);
}

err_t HttpRequest::onHeadersDone(
    httpc_state_t *connection, struct pbuf *hdr, u16_t hdrLen, u32_t contentLen)
{
    TRACE << "Content len:" << contentLen;
    m_contentLen = contentLen;
    
    // Optionally dump the header. For api.openweathermap.org, it is equal to the content.
    //TRACE << "Header:" << std::string(static_cast<char *>(hdr->payload), hdr->len);
    
    return ERR_OK;
}

err_t HttpRequest::receive(struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    m_content += std::string(static_cast<char *>(p->payload), p->len);

    TRACE << "Received block of" <<p->len <<"bytes";
    TRACE << "err=" << static_cast<int>(err);
    TRACE << "So far, we have" << m_content.size() << "bytes";

    return ERR_OK;
}

void HttpRequest::onTransferComplete(
    httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err)
{
    TRACE << "Content-Length of header:" << m_contentLen;
    TRACE << "Received" << m_content.size() << "bytes in total";
    TRACE << "httpc_result=" << httpc_result; // 0 if successful, 3 if the wifi is not connected
    TRACE << "rx_content_len=" <<rx_content_len;
    TRACE << "srv_res=" << srv_res; // 200 if succesful, 0 if the wifi is not connected
    TRACE << "err=" << static_cast<int>(err); // 0 if successful, -16 if the wifi is not connected
    m_complete = true;

    if (m_onCompleteCallback)
        m_onCompleteCallback(m_content);
}

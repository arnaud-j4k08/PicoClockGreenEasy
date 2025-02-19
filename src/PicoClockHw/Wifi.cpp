#include "Wifi.h"
#include "Utils/Trace.h"
#include "gpio.h"

#include <pico/cyw43_arch.h>
#include <boards/pico.h>

namespace
{
    const int MONITOR_CONNECTION_PERIOD_MS = 100;
}

Wifi::Status Wifi::m_connectResult = Wifi::Unknown;
alarm_id_t Wifi::m_monitorConnectionAlarm = -1;
std::list<std::function<void(bool success)>> Wifi::m_connectionFinishedCallbacks;


bool Wifi::init()
{
    TRACE << "cyw43_arch_init";
    if (cyw43_arch_init() != 0) 
    {
        TRACE <<"failed to initialize";
        return false;
    }
    TRACE << "Initialized successfully";

    cyw43_arch_enable_sta_mode();

    // On Pico non-W, cyw43_arch_init still succeeds and turns the onboard led on (as the cyw43 uses
    // pin 25 on Pico W). Therefore, turn the led off again. This does not have any effect on 
    // Pico W.
    TRACE << "Turn led off";
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, false);

    return true;
}

void Wifi::deinit()
{
    cyw43_arch_deinit();
}

bool Wifi::connectBlocking()
{
    TRACE << "cyw43_arch_wifi_connect_timeout_ms";
    int res =  
        cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);
    TRACE << "Result: " << res;
    return handleConnectResult(res);
}

bool Wifi::connectAsync(const std::function<void(bool)> &finishedCallback)
{
    TRACE << "cyw43_arch_wifi_connect_async with ssid=" <<WIFI_SSID;
    // I am not sure I am getting it right, but if WIFI_SSID and WIFI_PASSWORD are empty strings, 
    // cyw43_arch_wifi_connect_async seems to reuse previously passed values, even after a power 
    // cycle. This behavior is not documented.
    int res = cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    TRACE << "Result: " << res;

    if (res == 0) 
    {
        TRACE << "Connection was started successfully";

        m_connectionFinishedCallbacks.push_back(finishedCallback);

        if (m_monitorConnectionAlarm == -1)
            m_monitorConnectionAlarm = 
                add_alarm_in_ms(MONITOR_CONNECTION_PERIOD_MS, &monitorConnection, nullptr, false);
    } else
        TRACE << "Connection not started";

    return handleConnectResult(res);
}

int64_t Wifi::monitorConnection(alarm_id_t id, void *user_data)
{
    int res = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    
    if (res == CYW43_LINK_JOIN || res == CYW43_LINK_NOIP)
    {
        // Reschedule the same alarm to continue waiting for connection
        return -MONITOR_CONNECTION_PERIOD_MS * 1000;
    } else
    {
        // Call callbacks that were waiting for a connection and forget them.
        for (const auto &c : m_connectionFinishedCallbacks)
            c(res == CYW43_LINK_UP);
        m_connectionFinishedCallbacks.clear();

        // Do not reschedule
        m_monitorConnectionAlarm = -1;
        return 0;
    }
}

bool Wifi::handleConnectResult(int res)
{
    switch(res)
    {
        case PICO_OK:
            m_connectResult = OK;
            return true;
        case PICO_ERROR_NOT_PERMITTED:
            m_connectResult = NotAvailable;
            return false;
        default:
            m_connectResult = Unknown;
            return false;
    }
}

Wifi::Status Wifi::linkStatus()
{
    // If connection failed, cyw43_tcpip_link_status does not return the error, so use m_connectResult
    // instead.
    if (m_connectResult != OK && m_connectResult != Unknown)
        return m_connectResult;

//    TRACE << "cyw43_tcpip_link_status";
    int res = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
//    TRACE << "res:" << res;

    switch(res)
    {
        case CYW43_LINK_DOWN:
            return Down;
        case CYW43_LINK_JOIN:
            return Connecting;
        case CYW43_LINK_NOIP:
            return NoIp;
        case CYW43_LINK_UP:
            return Connected;
        case CYW43_LINK_FAIL:
            return ConnectionFailed;
        case CYW43_LINK_NONET:
            return NoNetworkFound;
        case CYW43_LINK_BADAUTH:
            return AuthenticationFailed;
        default:
            TRACE << "Unknown result:" << res;
            return Unknown;
    }
}

std::string Wifi::linkStatusToString(Status s)
{
    switch (s)
    {
#define STATUS(item) \
    case item:       \
        return #item;

        STATUS(Unknown)
        STATUS(OK)
        STATUS(NotAvailable)
        STATUS(Down)
        STATUS(Connecting)
        STATUS(NoIp)
        STATUS(Connected)
        STATUS(ConnectionFailed)
        STATUS(NoNetworkFound)
        STATUS(AuthenticationFailed)
    }

    return "";
}
void Wifi::disconnect()
{
    TRACE << "In Wifi::disconnect";
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
}

void Wifi::cycle_sta_mode()
{
    cyw43_arch_disable_sta_mode();
    TRACE << "In Wifi::cycle_sta_mode after disable sta mode";

    cyw43_arch_enable_sta_mode();

    // On Pico non-W, cyw43_arch_init still succeeds and turns the onboard led on (as the cyw43 uses
    // pin 25 on Pico W). Therefore, turn the led off again. This does not have any effect on 
    // Pico W.
    TRACE << "in Wifi::cycle_sta_mode after enable sta mode";
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
}
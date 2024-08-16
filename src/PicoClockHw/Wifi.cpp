#include "Wifi.h"
#include "Utils/Trace.h"
#include "gpio.h"

#include <pico/cyw43_arch.h>
#include <boards/pico.h>

Wifi::Status Wifi::m_connectResult = Wifi::Unknown;

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
    return true;
}

void Wifi::deinit()
{
    cyw43_arch_deinit();
}

bool Wifi::connectBlocking()
{
    // Do not bother connecting if no SSID is configured.
    if (strlen(WIFI_SSID) == 0)
    {
        m_connectResult = NotAvailable;
        return false;
    }

    TRACE << "cyw43_arch_wifi_connect_timeout_ms";
    int res =  
        cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);
    TRACE << "Result: " << res;

    switch(res)
    {
        case PICO_OK:
            m_connectResult = OK;
            return true;
        case PICO_ERROR_NOT_PERMITTED:
            // On Pico non-W, cyw43_arch_init succeeds, turns the onboard led on (as the cyw43 uses
            // pin 25 on Pico W) and cyw43_arch_wifi_connect_timeout_ms fails with this 
            // error. Therefore, turn the led off again.
            TRACE << "Turn led off";
            gpio_init(PICO_DEFAULT_LED_PIN);
            gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
            gpio_put(PICO_DEFAULT_LED_PIN, false);

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

    int res = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

    TRACE << "res:" << res;
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
#include "Gps.h"

#include "Utils/Trace.h"
#include "Utils/Trampoline.h"

#include <hardware/uart.h>
#include <hardware/gpio.h>
#include <sstream>
#include <iomanip>

namespace
{
    const auto GPS_UART = uart0;
    const int TIMEOUT_MS = 2000;

    // Convert NMEA absolute position to decimal degrees
    // "ddmm.mmmm" or "dddmm.mmmm" really is D+M/60,
    // then negated if quadrant is 'W' or 'S'
    std::string nmeaToDeg(const std::string &lat_lon, const std::string &orientation)
    {
        size_t dot = lat_lon.find(".");
        if (dot != std::string::npos)
        {
            int deg = std::stoi(lat_lon.substr(0, dot - 2));
            double min = std::stod(lat_lon.substr(dot - 2, std::string::npos));

            double pos = (double)deg + min / 60.0;
            pos *= ((orientation == "W" || orientation == "S") ? -1 : 1);

            std::stringstream output;
            output << std::fixed << std::setprecision(4) << pos;

            return output.str();
        }
        return "";
    }
}

Gps *Gps::m_instance = nullptr;

Gps::Gps()
{
    m_instance = this;
    uart_init(GPS_UART, 9600);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
}

Gps::~Gps()
{
    setEnabled(false);
    uart_deinit(GPS_UART);
    m_instance = nullptr;
}

void Gps::setEnabled(bool enabled)
{
    // Select correct interrupt for the UART we are using
    const int UART_IRQ = GPS_UART == uart0 ? UART0_IRQ : UART1_IRQ;

    if (enabled)
    {
        // Set up and enable the interrupt handlers
        irq_set_exclusive_handler(UART_IRQ, onUartRx);
        irq_set_enabled(UART_IRQ, true);        

        // Enable the UART to send interrupts on reception
        uart_set_irq_enables(GPS_UART, true/*has data*/, false/*needs data*/);

        // Set up timer to detect reception timeout
        resetTimeoutAlarm();
    } else
    {
        uart_set_irq_enables(GPS_UART, false/*has data*/, false/*needs data*/);
        irq_set_enabled(UART_IRQ, false);

        // Stop timer
        if (m_timeoutAlarm != -1)
        {
            cancel_alarm(m_timeoutAlarm);
            m_timeoutAlarm = -1;
        }
    }
}

void Gps::resetTimeoutAlarm()
{
    if (m_timeoutAlarm != -1)
        cancel_alarm(m_timeoutAlarm);

    MAKE_TRAMPOLINE(Gps, onTimeout, userPtrAtEnd);
    m_timeoutAlarm = add_alarm_in_ms(TIMEOUT_MS, onTimeout, this, true);
}

int64_t Gps::onTimeout(alarm_id_t)
{
    m_timeoutAlarm = -1;
    m_timeoutCallback();
    return 0; // Do not reschedule
}

void Gps::onUartRx()
{
    while (uart_is_readable(GPS_UART)) 
    {
        char c = uart_getc(GPS_UART);
        switch(c)
        {
            case 10: // LR: ignore
                break;
            case 13: // CR: Handle the received line
                TRACE << m_instance->g_receiveBuffer;
                m_instance->onMessage(m_instance->g_receiveBuffer);
                m_instance->g_receiveBuffer.clear();
                break;
            default: // Gather line characters
                m_instance->g_receiveBuffer.push_back(c);
                break;
        }
    }
}

void Gps::onMessage(const std::string &msg)
{
    // Postpone timeout timer
    resetTimeoutAlarm();

    // Decode RMC message 
    // (see https://content.u-blox.com/sites/default/files/products/documents/u-blox6_ReceiverDescrProtSpec_%28GPS.G6-SW-10018%29_Public.pdf)

    std::istringstream stream(msg);
    std::string msgId;
    if (!std::getline(stream, msgId, ','))
        return;
    if (msgId != "$GPRMC") return;

    std::string utcTime;
    if (!std::getline(stream, utcTime, ','))
        return;
    TRACE << "UTC time:" << utcTime;

    std::string status;
    if (!std::getline(stream, status, ','))
        return;
    if (status != "A") return; // Stop if data not valid

    std::string nmeaLatitude;
    if (!std::getline(stream, nmeaLatitude, ','))
        return;
    TRACE << "NMEA Latitude:" << nmeaLatitude;
    
    std::string latitudeOrientation;
    if (!std::getline(stream, latitudeOrientation, ','))
        return;
    TRACE << "Latitude Orientation:" << latitudeOrientation;

    std::string latDecimal = nmeaToDeg(nmeaLatitude, latitudeOrientation);
    TRACE << "Decimal Latitude:" << latDecimal;

    std::string nmeaLongitude;
    if (!std::getline(stream, nmeaLongitude, ','))
        return;
    TRACE << "NMEA Longitude:" << nmeaLongitude;
    
    std::string longitudeOrientation;
    if (!std::getline(stream, longitudeOrientation, ','))
        return;
    TRACE << "Longitude Orientation:" << longitudeOrientation;

    std::string lonDecimal = nmeaToDeg(nmeaLongitude, longitudeOrientation);
    TRACE << "Decimal Longitude:" << lonDecimal;

    // Skip fields we don't need
    for (int i = 0; i < 2; i++)
    {
        std::string s;
        if (!std::getline(stream, s, ','))
            return;
    }

    std::string date;
    if (!std::getline(stream, date, ','))
        return;
    TRACE << "Date:" << date;

    onDateTime(date, utcTime);
}

void Gps::onDateTime(const std::string &date, const std::string &time)
{
    if (date.size() < 6 || time.size() < 6)
        return;

    tm dt = {};
    float secWithMs = std::stof(time.substr(4));
    dt.tm_sec = secWithMs;
    int ms = secWithMs * 1000 - dt.tm_sec * 1000;
    dt.tm_min = std::stoi(time.substr(2, 2));
    dt.tm_hour = std::stoi(time.substr(0, 2));
    dt.tm_mday = std::stoi(date.substr(0, 2));
    dt.tm_mon = std::stoi(date.substr(2, 2)) - 1; // tm::tm_mon is 0-based.
    dt.tm_year = std::stoi(date.substr(4, 2)) + 100; // tm::tm_year is "years since 1900"

    TRACE << "Received GPS date/time:" << dt;
    TRACE << "ms:" << ms;

    if (m_timeCallback)
        m_timeCallback(mktime(&dt), ms);
}
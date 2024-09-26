#include "Rtc.h"

#include "gpio.h"
#include "Utils/Trace.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/sync.h>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace
{
    const auto I2C_PORT = i2c1;
    const uint8_t DEVICE_ADDRESS = 0x68;
    const uint TIMEOUT_US = 100000; // 100 ms

    uint8_t fromBcd(uint8_t value, int min, int max)
    {
        int result = (value >> 4) * 10 + (value & 0x0F);
        if (result < min)
            result = min;
        if (result > max)
            result = max;
        return result;
    }

    uint8_t toBcd(uint8_t value)
    {
        return ((value / 10) << 4) + (value % 10);
    }
}

Rtc *Rtc::m_instance = nullptr;

Rtc::Rtc()
{
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);

    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    TRACE << "Initialize the temperature filter";
    m_tempFilter = std::make_unique<MovingAverage<32>>(rawTemperature());

    m_instance = this;
}

Rtc::~Rtc()
{
    m_instance = nullptr;
    i2c_deinit(I2C_PORT);
}

bool Rtc::read(tm &dateTime) const
{
    // Fail if another method is currently communicating with the DS3231
    if (m_communicating)
        return false;

    // Read registers from the DS3231
    // See https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf for details
    uint8_t registerNumber = 0x00;
    if (i2c_write_timeout_us(
        I2C_PORT, DEVICE_ADDRESS, &registerNumber, 1, true, TIMEOUT_US) != 1)
    {
        TRACE << "i2c_write_timeout_us failed";
        return false;
    }
    unsigned char buffer[7];
    if (i2c_read_timeout_us(
        I2C_PORT, DEVICE_ADDRESS, buffer, sizeof(buffer), false, TIMEOUT_US) != sizeof(buffer))
    {
        TRACE << "i2c_read_timeout_us failed";
        return false;
    }

    dateTime.tm_sec = fromBcd(buffer[0], 0, 59);
    dateTime.tm_min = fromBcd(buffer[1], 0, 59);
    dateTime.tm_hour = fromBcd(buffer[2], 0, 23);
    dateTime.tm_wday = fromBcd(buffer[3], 1, 7) - 1; // Unlike in the DS3231, tm::tm_wday is 0-based
    dateTime.tm_mday = fromBcd(buffer[4], 1, 31);
    dateTime.tm_mon = fromBcd(buffer[5] & 0x1F, 1, 12) - 1; // Unlike in the DS3231, tm::tm_mon is 0-based.
    dateTime.tm_year = fromBcd(buffer[6], 0, 99) + 100;    // tm::tm_year is "years since 1900"

    if (buffer[5] & 0x80)
        dateTime.tm_year += 100; // Century flag set

    return true;
}

bool Rtc::write(const tm &dateTime)
{
    // Fail if another method is currently communicating with the DS3231
    if (m_communicating)
        return false;

    unsigned char buffer[8];
    buffer[0] = 0;  // Start writing at register 0
    buffer[1] = toBcd(dateTime.tm_sec);
    buffer[2] = toBcd(dateTime.tm_min);
    buffer[3] = toBcd(dateTime.tm_hour);
    buffer[4] = toBcd(dateTime.tm_wday + 1); // Unlike in the DS3231, tm::tm_wday is 0-based
    buffer[5] = toBcd(dateTime.tm_mday);

    buffer[6] = toBcd(dateTime.tm_mon + 1); // Unlike in the DS3231, tm::tm_mon is 0-based.
    buffer[7] = toBcd(dateTime.tm_year % 100);

    // Set the century flag if the year is >= 2100. (tm::tm_year is in "years since 1900")
    if (dateTime.tm_year >= 200)
        buffer[6] |= 0x80;

    int written = i2c_write_timeout_us(I2C_PORT, DEVICE_ADDRESS, buffer, sizeof(buffer), false, TIMEOUT_US);
    if (written == sizeof(buffer))
    {
        TRACE << "i2c_write_timeout_us successful";
        return true;
    } else 
    {
        TRACE << "i2c_write_timeout_us failed";
        return false;
    }
}

float Rtc::temperature()
{
    // Defer temperature measurement to the main loop, as it takes too long for interrupts.
    m_requestTemp = true;

    // As the measured temperature is often hesitating between two values separated by 0.25°, filter
    // using a moving average. This also provides a higher resulting precision.
    float temp = m_tempFilter->get();

    TRACE << "Temperature: " << temp;
    return temp;
}

void Rtc::onSecond()
{
    // Do nothing if the instance does not exist yet or no temperature measurement is requested
    if (m_instance == nullptr || !m_instance->m_requestTemp)
        return;

    // Acknowledge the request
    m_instance->m_requestTemp = false;

    TRACE << "Measure";
    float currentTemp = m_instance->rawTemperature();

    // Prevent access to member variables from interruptions
    uint32_t interrupts = save_and_disable_interrupts();

    // Update the moving average with this measurement
    m_instance->m_tempFilter->put(currentTemp);

    // Interrupts may access member variables again
    restore_interrupts(interrupts);
}

float Rtc::rawTemperature()
{
    // Get temperature from RTC
    m_communicating = true;
    uint8_t registerNumber = 0x11;
    int result = i2c_write_timeout_us(
        I2C_PORT, DEVICE_ADDRESS, &registerNumber, 1, true, TIMEOUT_US);
    if (result != 1)
    {
        m_communicating = false;
        TRACE << "i2c_write_timeout_us returned" << result;
        return NAN;
    }
    unsigned char buffer[2];
    if (i2c_read_timeout_us(I2C_PORT, DEVICE_ADDRESS, buffer, sizeof(buffer), false, TIMEOUT_US) != 
        sizeof(buffer))
    {
        m_communicating = false;
        TRACE << "i2c_read_timeout_us failed";
        return NAN;
    }
    m_communicating = false;

    // Calculate temperature as floating point number
    float currentTemp = buffer[0];
    float fractional = (buffer[1] >> 6) * 0.25;
    if (currentTemp >= 0)
        currentTemp += fractional;
    else
        currentTemp -= fractional;
    TRACE << "Measured temperature:" <<std::fixed << std::setprecision(2) << currentTemp;

    return currentTemp;
}
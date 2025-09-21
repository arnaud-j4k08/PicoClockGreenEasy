#include "Temperature.h"
#include "Utils/Trace.h"
#include "Clock.h"

#include <cmath>

#include "PicoClockHw/Platform.h"

bool Temperature::isAvailable() const
{
    return clock().hasRtc();
}

void Temperature::activate()
{
    // Toggle the temperature format
    modifySettings().useCelsius = !settings().useCelsius;
}

void Temperature::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    // Read the temperature from the RTC once per second or if it has not been measured yet.
    if (
        clock().rtc() != nullptr && 
        (clock().tickCount() == 0 || std::isnan(m_lastMeasuredTempCelsius)))
    {
        m_lastMeasuredTempCelsius = clock().rtc()->temperature();

#ifdef RTC_TEMP_CALIB
        if (!std::isnan(m_lastMeasuredTempCelsius))
        {
            // RTC Temperature calibration 
            m_lastMeasuredTempCelsius += RTC_TEMP_CALIB;
        }
#endif
    }

    // On full refresh, clear the frame and draw the static elements (dot and degree sign)
    if (fullRefresh)
    {
        frame.clear();

        // Draw the dot
        frame.putPixel(12, 6, true);

        // Draw the degree sign
        frame.putPixel(20, 0, true);
        frame.putPixel(19, 1, true);
        frame.putPixel(21, 1, true);
        frame.putPixel(20, 2, true);
    }

    // Draw the C or F indicator
    frame.putIndicator(Bitmap::C, settings().useCelsius);
    frame.putIndicator(Bitmap::F, !settings().useCelsius);

    // Do not display anything else if the temperature could not be measured
    if (std::isnan(m_lastMeasuredTempCelsius))
        return;

    // Convert to Fahrenheit if needed
    float displayedTemp = m_lastMeasuredTempCelsius;
    if (!settings().useCelsius)
        displayedTemp = displayedTemp * 9 / 5 + 32;

    // Draw or remove the minus sign and use the absolute value if negative
    frame.drawRectangle(0, 3, 1, 3, displayedTemp < 0);
    if (displayedTemp < 0)
        displayedTemp = -displayedTemp; 

    // Draw the temperature value with or without animation
    int decimalChar = '0' + static_cast<int>(displayedTemp * 10) % 10;
    if (settings().digitAnimation)
    {
        AnimatedChar::render2DigitsInt(
            m_digit1, m_digit2, frame, fullRefresh, static_cast<int>(displayedTemp));
        m_digit3.renderChar(
            frame, 
            fullRefresh, 
            decimalChar);
    } else
    {
        if (displayedTemp != m_displayedTemp || fullRefresh)
        {
            TRACE <<"fullRefresh: " << fullRefresh;
            m_displayedTemp = displayedTemp;
            TRACE <<"Display temp: " << displayedTemp;
            frame.drawRectangle(2, 0, 10, 6, false);
            frame.setFont(&classicFont);
            frame.draw2DigitsInt(2, 0, static_cast<int>(displayedTemp));
            frame.drawRectangle(14, 0, 17, 6, false);
            frame.drawChar(14, 0, decimalChar);
        }
    }
}

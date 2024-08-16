#include "Temperature.h"
#include "Utils/Trace.h"
#include "Clock.h"

#include <cmath>

bool Temperature::isAvailable() const
{
    return clock().hasRtc();
}

void Temperature::activate()
{
    // Toggle the temperature format and refresh
    modifySettings().useCelsius = !settings().useCelsius;
    forceRefresh();
}

void Temperature::renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    if ((!fullRefresh && clock().tickCount() != 0) || clock().rtc() == nullptr) return;

    TRACE << "Get temperature";
    float temp = clock().rtc()->temperature();

    if (std::isnan(temp))
        return;

    if (!settings().useCelsius)
        temp = temp * 9 / 5 + 32;

    char tempString[5];
    sprintf(tempString, "%4.1f", temp);
    TRACE <<SetAutoSpace(false) << "tempString: '" << tempString << "'";

    frame.clear();
    frame.setFont(&classicFont);

    if (temp < 0)
    {
        // Draw a minus sign
        frame.drawRectangle(0, 3, 1, 3, true);
        
        // Use the absolute value in the rest of the function
        temp = -temp; 
    }

    frame.drawChar(14, 0, tempString[3]);
    frame.putPixel(12, 6, true);
    tempString[2] = 0;
    frame.drawText(2, 0, tempString);

    // Draw the degree sign
    frame.putPixel(20, 0, true);
    frame.putPixel(19, 1, true);
    frame.putPixel(21, 1, true);
    frame.putPixel(20, 2, true);

    if (settings().useCelsius)
        frame.putIndicator(Bitmap::C, true);
    else
        frame.putIndicator(Bitmap::F, true);
}

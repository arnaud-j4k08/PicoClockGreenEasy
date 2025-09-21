#include "fonts.h"
#include "ClockUi.h"
#include "Utils/Trace.h"

#include "PicoClockHw/Platform.h"
#include "PicoClockHw/Wifi.h"

int main() 
{
    Platform::initStdIo();
    Platform::enableWatchdog();


    TRACE << "Clock UI";
    ClockUi ui;

    TRACE << "Wifi::init()";
    if (Wifi::init())
        ui.onWifiInited();

    TRACE <<"Start the loop\n";
    Platform::runMainLoop();

    // Not reachable for the moment, but a shutdown function may be added later.
    Wifi::deinit();

    return 0;
}
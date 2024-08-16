#include "fonts.h"
#include "ClockUi.h"
#include "Utils/Trace.h"

#include "PicoClockHw/Platform.h"
#include "PicoClockHw/Wifi.h"

int main() 
{
    Platform::initStdIo();

    // Can be enabled to delay startup in order to debug
#if 0    
    for (int i = 5; i > 0;i--)
    {
        sleep_ms(1000);
        std::cout << i << std::endl;
    }
#endif
    TRACE << "Clock UI";
    ClockUi ui;

    TRACE << "Wifi::init";
    Wifi::init();

    TRACE << "Connect to wifi\n";

    if (Wifi::connectBlocking())
        ui.startNtpRequest();

    TRACE <<"Start the loop\n";
    Platform::runMainLoop();

    // Not reachable for the moment, but a shutdown function may be added later.
    Wifi::deinit();

    return 0;
}
#include "fonts.h"
#include "ClockUi.h"
#include "Utils/Trace.h"
#include "PicoClockHw/HttpRequest.h"
#include "PicoClockHw/Platform.h"
#include "PicoClockHw/Wifi.h"

int main() 
{
    Platform::initStdIo();

    // Can be enabled to delay startup in order to debug
//#if 0
    for (int i = 15; i > 0;i--)
    {
        sleep_ms(1000);
        std::cout << i << std::endl;
    }
//#endif
    TRACE << "Clock UI";
    ClockUi ui;

    TRACE << "Wifi::init()";
    if (Wifi::init())
        ui.onWifiInited();


    // TODO: Remove this, this is only for testing. Wait until the Wifi connection initiated in
    // Clock is completed.
    TRACE << "Connected to Wifi\n";
    while (Wifi::linkStatus() != Wifi::Connected)
        ;
    TRACE << "Connected!\n";
    
    TRACE <<"Start the loop\n";
    Platform::runMainLoop();

    // Not reachable for the moment, but a shutdown function may be added later.
    Wifi::deinit();

    return 0;
}
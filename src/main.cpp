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
    
    // TODO: Only for testing. Perform the request and output the result.
    //TRACE << "Creating HttpRequest object called req \n";
    //HttpRequest req;       // this is creating a class object called req
    //TRACE << "Issuing HttpRequest Start request\n ";
    //req.start();
    //while (!req.isComplete())
    //    ;
    //std::cout << "HTTP request result: " << req.content() <<std::endl;

        // TODO: declared here only for testing
    //TRACE << "In Main, calling Weather2 weather2\n";    
    //Weather2 weather2;  // sets callback 
    //TRACE << "In Main, after Weather2 weather2, calling sync\n";
    //weather2.sync();
    //TRACE << "In Main, After weather2 sync\n";


    TRACE <<"Start the loop\n";
    Platform::runMainLoop();

    // Not reachable for the moment, but a shutdown function may be added later.
    Wifi::deinit();

    return 0;
}
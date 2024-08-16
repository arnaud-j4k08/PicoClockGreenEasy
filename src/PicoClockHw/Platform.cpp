#include "Platform.h"
#include "Rtc.h"
#include <pico/stdlib.h>

void Platform::initStdIo()
{
    stdio_init_all();
}

void Platform::runMainLoop()
{
    while (1)
    {
        sleep_ms(1000);
        Rtc::onSecond();
    }
}

int Platform::getCharNonBlocking()
{
    return getchar_timeout_us(0);
}

uint64_t Platform::timeUs()
{
    return time_us_64();
}
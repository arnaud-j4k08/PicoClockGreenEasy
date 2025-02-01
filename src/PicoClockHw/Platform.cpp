#include "Platform.h"
#include "Rtc.h"
#include <pico/stdlib.h>
#include <pico/rand.h>

void Platform::initStdIo()
{
    stdio_init_all();

    // Give the highest possible priority to the USB controller IRQ, to prevent the output buffer
    // from getting full when using TRACE within an interrupt handler.
    irq_set_priority(USBCTRL_IRQ, 0);
}

void Platform::runMainLoop()
{
    while (1)
    {
        sleep_ms(1000);
        tight_loop_contents();
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

uint32_t Platform::randomNumber32()
{
    return get_rand_32();
}
#include "Platform.h"
#include "Rtc.h"
#include <iostream>
#include <pico/stdlib.h>
#include <pico/rand.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>

void Platform::initStdIo()
{
    stdio_init_all();

    // Give the highest possible priority to the USB controller IRQ, to prevent the output buffer
    // from getting full when using TRACE within an interrupt handler.
    irq_set_priority(USBCTRL_IRQ, 0);
}

void Platform::enableWatchdog()
{
    if (watchdog_caused_reboot())
    {
        sleep_ms(1000);
        std::cout 
            << "The program has crashed or has been aborted. Press 'b' to reboot to bootsel mode." 
            << std::endl;
        for (int i = 2; i > 0; i--)
        {
            sleep_ms(1000);
            std::cout << i  <<std::endl;
            if (getCharNonBlocking() == 'b') 
            {
                reset_usb_boot(0, 0);
            }
        }
    }

    watchdog_enable(5000, true /*pause_on_debug*/);
}

void Platform::runMainLoop()
{
    while (1)
    {
        sleep_ms(1000);
        watchdog_update();
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
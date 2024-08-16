#include "Buzzer.h"
#include "gpio.h"
#include "Utils/Trampoline.h"

#include <hardware/gpio.h>

Buzzer::Buzzer()
{
    gpio_init(BUZZ);
    gpio_set_dir(BUZZ, GPIO_OUT);
}

Buzzer::~Buzzer()
{
    if (m_stopBeepAlarm != -1)
        cancel_alarm(m_stopBeepAlarm);
}

void Buzzer::beepForMs(int delay)
{
    gpio_put(BUZZ, true);

    if (m_stopBeepAlarm != -1)
        cancel_alarm(m_stopBeepAlarm);

    MAKE_TRAMPOLINE(Buzzer, stopBeep, userPtrAtEnd);
    m_stopBeepAlarm = add_alarm_in_ms(delay, stopBeep, this, true /* fire if past*/);
}

int64_t Buzzer::stopBeep(alarm_id_t id)
{
    gpio_put(BUZZ, false);

    m_stopBeepAlarm = -1;
    
    return 0; // Do not reschedule
}
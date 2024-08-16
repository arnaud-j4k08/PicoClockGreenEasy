#pragma once

#include <pico/time.h>
#include <cstdint>

class Buzzer
{
public:
    Buzzer();
    ~Buzzer();

    void beepForMs(int delay);

private:
    int64_t stopBeep(alarm_id_t id);

    alarm_id_t m_stopBeepAlarm = -1;
};
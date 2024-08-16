#pragma once

#include <map>
#include <functional>
#include <cstdint>
#include <pico/time.h>

class Button
{
public:
    Button(unsigned int gpio);
    ~Button();

    void setPressedCallback(std::function<void()> f);
    void setRepeatCallback(std::function<void()> f, int delayMs);

private:
    static void dispatcher(unsigned int gpio, uint32_t events);
    int64_t debounceCallback(alarm_id_t id);
    int64_t repeatCallback(alarm_id_t id);

    unsigned int m_gpio;
    static std::map<unsigned int, Button *> m_buttonByGpio;

    std::function<void()> m_pressedCallback;
    std::function<void()> m_repeatCallback;

    int m_repeatDelay = 0;
    alarm_id_t m_repeatAlarm = -1;
    alarm_id_t m_debounceAlarm = -1;
};

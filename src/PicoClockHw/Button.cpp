#include "Button.h"
#include "Utils/Trace.h"
#include "Utils/Trampoline.h"

#include <hardware/gpio.h>

namespace {
    const int DEBOUNCE_DELAY_MS = 10;
}

std::map<unsigned int, Button *> Button::m_buttonByGpio;

Button::Button(unsigned int gpio) : m_gpio(gpio)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
    gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true, dispatcher);

    m_buttonByGpio.insert(std::make_pair(gpio, this));
}

Button::~Button()
{
    m_buttonByGpio.erase(m_gpio);
    
    cancel_alarm(m_repeatAlarm);
    cancel_alarm(m_debounceAlarm);
}

void Button::setPressedCallback(std::function<void()> f)
{
    m_pressedCallback = f;
}

void Button::setRepeatCallback(std::function<void()> f, int delayMs)
{
    m_repeatCallback = f;
    m_repeatDelay = delayMs;
}

void Button::dispatcher(unsigned int gpio, uint32_t events)
{
    auto it = m_buttonByGpio.find(gpio);

    if (it == m_buttonByGpio.end())
        return;

    Button &obj = *it->second;

    // To debounce, delay the actual processing using an alarm.
    if (obj.m_debounceAlarm != -1) // Note: calling cancel_alarm with -1 would crash
    {
        TRACE << "Cancel alarm " << obj.m_debounceAlarm;
        cancel_alarm(obj.m_debounceAlarm);
    }
    MAKE_TRAMPOLINE(Button, debounceCallback, userPtrAtEnd);
    obj.m_debounceAlarm = 
        add_alarm_in_ms(DEBOUNCE_DELAY_MS, debounceCallback, &obj, true /* fire if past*/);
    TRACE <<"End of dispatcher\n";
}

int64_t Button::debounceCallback(alarm_id_t id)
{
    if (!gpio_get(m_gpio))
    {
        // Button pressed, call the user callback
        TRACE << "Call user callback\n";
        m_pressedCallback();
        
        // If a repeat callback is installed, start an alarm for it
        if (m_repeatCallback)
        {
            TRACE << "Current repeat alarm: " << m_repeatAlarm;
            MAKE_TRAMPOLINE(Button, repeatCallback, userPtrAtEnd)
            m_repeatAlarm = add_alarm_in_ms(m_repeatDelay, repeatCallback, this, false);
        }
    } else
    { 
        // Button released. Cancel the repetition alarm.
        TRACE << "Repeat alarm to cancel: " << m_repeatAlarm;
        if (m_repeatAlarm != -1)
        {
            bool ok = cancel_alarm(m_repeatAlarm);
            TRACE <<"Result of cancel_alarm: "<<ok;
        }
        m_repeatAlarm = -1;
    }

    // Do not reschedule
    m_debounceAlarm = -1;
    return 0; 
}

int64_t Button::repeatCallback(alarm_id_t id)
{
    if (gpio_get(m_gpio))
    { 
        // Button not pressed anymore. Do not reschedule the alarm
        TRACE << "Not rescheduled\n";
        return 0;
    } else
    {
        m_repeatCallback();

        // Reschedule the same alarm this many us from the time the alarm was previously 
        // scheduled to fire
        return -m_repeatDelay * 1000;
    }
}

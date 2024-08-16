#include "Settings.h"
#include "PicoClockHw/Flash.h"
#include "Utils/Trace.h"

Settings::Settings()
{
    Flash::attach(reinterpret_cast<uint8_t *>(&m_values), sizeof(m_values));
    Flash::read();

    TRACE << "countdownStartSec" << m_values.countdownStartSec;
}

Settings::Values &Settings::modify()
{
    Flash::scheduleWrite();

    return m_values;
}
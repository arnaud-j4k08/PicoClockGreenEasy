#include "UiTexts.h"

namespace
{
    const char *g_textTable[static_cast<unsigned int>(TextId::TextCount)] =
        {
            "Alarms",
            "Wifi: ",
            "Options",
            "Exit",

            "Unknown",
            "OK",
            "Not available",
            "Down",
            "Connecting",
            "No IP",
            "Connected",
            "Connection failed",
            "No network found",
            "Authentication failed",

            "Al 1: ",
            "Al 2: ",
            "Loud",
            "Gradual",
            "Skip next alarm: ",

            "Auto light: ",
            "Time format: ",
            "24h",
            "12h",
            "Hourly chime: ",
            "Auto scroll: ",
            "On",
            "Off",
            "Day",

            "Stopwatch",
            "Reset",
            "Countdown",
            "Set",
    };
}

std::string uiText(TextId id)
{
    return g_textTable[static_cast<unsigned int>(id)];
}
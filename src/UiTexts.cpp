#include "UiTexts.h"

namespace
{
    const char *g_textTable[static_cast<unsigned int>(TextId::TextCount)] =
        {
            "Alarms",
            "Options",
            "Exit",

            "Unknown",
            "OK",
            "Not available",
            "Down",
            "Joining",
            "No IP",
            "Connected",
            "Connection failed",
            "No network found",
            "Authentication failed",

            "Next: ",
            "Al 1: ",
            "Al 2: ",
            "Loud",
            "Gradual",
            "Skip next alarm: ",
            "Once",
            "Weekly",

            "Auto light: ",
            "Time format: ",
            "24h",
            "12h",
            "Date format: ",
            "Hourly chime: ",
            "Auto scroll: ",
            "Brightness: ",
            "Dark brightness: ",
            "Dim brightness: ",
            "Max brightness: ",
            "On",
            "Off",
            "Day",

            "Stopwatch",
            "Reset",
            "Countdown",
            "Set",

            "MM-DD",
            "MM/DD",
            "DD-MM",
            "DD/MM",
            "DD.MM",

            "Clock sync",
            "Source: ",
            "Sync now",
            "Daily sync time: ",
            "Last sync: ",
            "Last drift: ",
            "Wifi: ",

            "RTC",
            "NTP",
            "GPS",

        // Weather related Text.  Associated with WeatherInfo structure 
            "Weather",                  
            "Conditions: ",             
            "Temp: ",                   
            "Pressure: ",              
            "Humidity: ",               
            "Wind Speed: ",             
            "Wind Dir: ",               
            "Wind: ",                   
            "Sunrise: ",                
            "Sunset: ",                 
            "Weather for: ",            
            "Update Weather Now",       
            "Last Update: ",                        

    };
}

std::string uiText(TextId id)
{
    return g_textTable[static_cast<unsigned int>(id)];
}
![image](Clock.jpeg)

# Pico Clock Green & Easy
This project is an easy-to-use firmware for the Waveshare Pico-Clock-Green written in C++. See also the [Introduction video on YouTube](https://youtu.be/PO-PY_PxIrI). For any question, feel free to post in the [Discussion section](https://github.com/arnaud-j4k08/PicoClockGreenEasy/discussions).

# Features
## User features
- basic (also present in Waveshare's demo firmware):
    - auto scroll
    - 2 alarms (with multiple weekdays selection)
    - time format 12h or 24h
    - room temperature (using the RTC sensor)
    - countdown
    - stopwatch
    - hourly chime
    - auto light, to automatically adjust the brightness of the leds depending on the ambient light
- time and date synchronization:
    - NTP synchronization over Wi-Fi (requires a development environment to set the SSID and password)
    - GPS synchronization (requires an additional GPS module)
    - automatic daily re-synchronization
- additional:
    - menu based user interface with horizontal and vertical scrolling
    - instant start up: no splash screen or animation, just power the device and you have a clock
    - 3 time display styles: hour:min:sec, hour:min + bar (the bar is a kind of horizontal hourglass to show seconds), hour:min
    - automatic daylight saving time observation (currently only for European Union and USA)
    - persistent saving of clock settings to flash memory
    - optional hourly chime activation using the ambient light sensor
    - configurable date format: 5 formats with different orders and separators
    - configurable brightness: 
      - if auto light is disabled, the brightness can be set as a percentage
      - if auto light is enabled, the brightness can be defined at three ambient light points between which the firmware will interpolate: dark (no ambient light), dim (10% ambient light), max (maximum ambient light)
    - brightness boost: if auto light is enabled and the ambient light is below 10%, the brightness is temporarily increased by 20% during 5 seconds after a user input, so that the display is more readable when setting something during the night
- alarms:
    - next alarm: displays next time and weekday when an alarm will ring, so that the user can quickly check if the alarm was set correctly before sleeping
    - weekly/once: If one or more weekdays are selected, the alarm will ring every week on these days. If no weekday is selected, the alarm will ring when the defined time is reached and disable itself.
    - skip next alarm: e.g. if you woke up before the alarm time or the next day is a national holiday, activate this function and the next alarm (and only this one) will be skipped. This is shown by the slow blinking of the "Alarm On" indicator.
    - gradual alarm mode that progressively increases the duration of beeps to wake up the user gently
- optional weather:
    - current conditions, temperature, wind speed, wind direction, humidity, pressure, sunrise, sunset
    - updated every 30 minutes, with menu option to update on-demand
    - requires Wifi connection, and no-cost subscription to OpenWeatherMap.org
    - if left in weather submenu, automatically scroll between temperature, wind speed, wind direction, and weather conditions     

## Technical features
- support for Pico and Pico W
- written in C++ from scratch (not based on Waveshare's demo code)
- intuitive display frame buffer structure, with one bit per pixel in displaying order
- 3 fonts (4x5 pixels monospaced, 4x7 pixels monospaced and 3x7 pixels proportional), directly modifiable in the source code
- hardware abstraction layer to facilitate porting to other platforms and adding unit tests
- led matrix controller fully driven by DMA and PIO to release the CPU and provide a more stable display (can be disabled at build time)
- use of PWM to control display brightness
- use of interrupts for buttons, with debouncing


# Installation
## From binaries

The repository contains the precompiled PicoClockGreenEasy.uf2 file that can be used to easily install the firmware on your Pico-Clock-Green device.

### Using the BOOTSEL button
- Hold the BOOTSEL button of the Pico, connect the USB interface of the Pico to your computer, then release the button.
- Copy the uf2 into the Pico drive. The Pico will run the firmware directly.

### Using picotool
If you have the Pico build environment, the firmware can be installed using picotool:
- Connect the USB interface of the Pico to your computer
- Reboot to BOOTSEL mode using picotool. This works only if the program that is running on the Pico is cooperative (e.g. if it uses stdio_usb), which is the case with this firmware.
```
picotool reboot -f -u
```
- Load and execute the firmware
```
picotool load -x ./PicoClockGreenEasy.uf2
```

## Building from the source code

- Install the Pico development environment, e.g. by following https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf
- clone this repository locally
- configure the firmware by modifying the UserConfig.cmake file, especially if you want to use NTP or Weather synchronization over Wi-Fi
- enter the corresponding directory and configure the project
```
cmake . -Bbuild
```
- build
```
cmake --build build
```

The created build/PicoClockGreenEasy.uf2 can now be transferred to the Pico by following the instructions of the previous section.

# User manual

## Concept

The whole user interface is organized as a menu, with some functions opening sub-menus. Vertical scrolling helps users understand that they are navigating in a menu, whereas horizontal scrolling is used to display long texts, so that they do not need to be abbreviated.

The three buttons follow the assignment defined by Waveshare (from top to bottom):
- enter / set
- move up in menu
- move down in menu

Whenever a value is blinking, it can be modified using this button assignment:
- finish setting / set next value
- increase value, making bigger steps repeatedly on long-press
- decrease value, making bigger steps repeatedly on long-press

Additionally, the buttons behave slightly differently when editing the weekdays of an alarm:
- finish setting alarm weekdays
- select next weekday
- enable/disable alarm on this weekday

## Menu structure
This is the full definition of the menu structure. Use the "enter/set" button to access functions/values on the right.

- time in hour:min:sec style &rarr; set hour &rarr; set min
- time in hour:min:bar style &rarr; set hour &rarr; set min
- time in hour:min style &rarr; set hour &rarr; set min
- date &rarr; set year &rarr; set month &rarr; set day
- temperature: toggle Celsius/Fahrenheit
- alarms (with next alarm time and weekday if an alarm is activated): enter submenu
    - (if an alarm is activated) skip next alarm: toggle on/off
    - alarm 1 &rarr; set mode &rarr; set hour &rarr; set min &rarr; set weekdays
    - alarm 2 &rarr; set mode &rarr; set hour &rarr; set min &rarr; set weekdays
    - exit: leave submenu
- countdown: enter submenu
    - countdown: start/stop 
    - set &rarr; reset countdown to start time, then set min &rarr; set sec
    - exit: leave submenu
- stopwatch: enter submenu
    - stopwatch: start/stop
    - reset: reset stopwatch to zero
    - exit: leave submenu
- clock sync: enter submenu
    - source -> set sync source (RTC, NTP or GPS)
    - sync now: synchronize with the selected source
    - daily sync time (show when the clock automatically re-synchronizes, no actual function)
    - last sync (show timestamp of last synchronization and the used source, no actual function)
    - last drift (show how much the internal clock had drifted last time it was synchronized, no actual function)
    - wifi (show wifi connection status, no actual function)
    - exit: leave submenu
- (if auto light is off) options &rarr; set auto scroll &rarr; set time format &rarr; set date format &rarr; set hourly chime &rarr; set auto light -> set brightness
- (if auto light is on) options &rarr; set auto scroll &rarr; set time format &rarr; set date format &rarr; set hourly chime &rarr; set auto light -> set dark brightness -> set dim brightness -> set max brightness
- weather (optional): enter submenu
    - update weather now (issue call to OpenWeatherMap API for latest weather information)
    - conditions (clear sky, overcast, rain, etc.)
    - temp (display current temperature in Celsius/Fahrenheit per temperature toggle and OpenWeatherMap selection)
    - wind speed (display current wind speed in mph/kmh per temperature toggle and OpenWeatherMap selection)
    - wind direction (display compass degrees and cardinal direction)
    - humidity (display percent humidity)
    - pressure (display atmospheric pressure in hPa)
    - sunrise (display sunrise in local time)
    - sunset (display sunset in local time)
    - last update time (show timestamp of last weather update)
    - exit: leave submenu


## Clock synchronization
At start-up, the clock synchronizes itself with the source selected in the "clock sync" submenu. It then synchronizes itself again every day at a random time generated at start-up. A synchronization can also be triggered manually using the "sync now" function. Whenever a synchronization is in progress, the °F and °C indicators blink slowly.

NTP and GPS requires some configuration:

### Using NTP synchronization

If you have a Pico W, you can use NTP to synchronize date/time at start-up. For the moment, this requires a build environment, as the Wi-Fi SSID and password need to be configured at build time. Additional possibilities may get added in future versions.

This configuration is done by setting the WIFI_SSID and WIFI_PASSWORD macros in the UserConfig.cmake file (between the escaped quotes). Additionally, the UTC offset also needs to be set in UTC_OFFSET, as the NTP server provides UTC time and does not know where you are located. After configuring, follow the steps of the "Building from the source code" section above. 

When running the firmware, go the the "clock sync" submenu and set the sync source to "NTP". You can use the "wifi" function to check if your settings are working. Note that the Wi-Fi connection will only be attempted when actually synchronizing (at start-up or manually). The connection is then kept open.


### Using GPS synchronization

If you don't have a Pico W, or want to benefit from a synchronized clock without having to setup a Wi-Fi connection, you can connect an additional GPS module that will provide date and time information to the firmware. 

I tested this feature using a NEO-6M GPS module. It should work with other modules that have an UART interface that uses the NMEA protocol. Here is the pin wiring I used:
|GPS module|Pico    |
|----------|--------|
|VCC       |3V3(OUT)|
|RX        |GP0     |
|TX        |GP1     |
|GND       |GND     |

When running the firmware, go to the "clock sync" submenu and set the sync source to "GPS".


## Configuring daylight saving time

The clock can be configured to automatically observe daylight saving time. For the moment, this requires a build environment, as the UTC offset and location need to be configured at build time. Currently, only the European Union and United States variants of daylight saving time are supported.

The configuration is done by setting the UTC_OFFSET and DST_LOCATION macros in the UserConfig.cmake file. After configuring, follow the steps of the "Building from the source code" section above. At runtime, the clock will then automatically advance when daylight saving time begins and change back to regular time when it ends.


## Setting brightness

### Manual setting

- scroll to the "Options" function
- press "set" several times to reach the "auto light" setting
- press "up" or "down" to set "auto light" to "off"
- press "set"
- use "up" or "down" to the set the brightness percentage
- press "set"

### Automatic setting

With auto light, the brightness is automatically adjusted depending on the ambient light. The mapping from ambient light to brightness can be set at three points to fit to all usage conditions.

- scroll to the "Options" function
- press "set" several times to reach the "auto light" setting
- press "up" or "down" to set "auto light" to "on"
- press "set" (for brightness settings, the "brightness boost" is temporarily disabled)
- go to a completely dark room and use the "up" or "down" buttons to set the "dark brightness" percentage (to make the display very dark for sleeping, this value can also be negative, but the display will never be completely turned off, as a brightness of 0% or less results in a effective brightness of 0.1%)
- press "set"
- go to a not-so-dark room (or turn on the light) and use the "up" or "down" buttons to set the "dim brightness" percentage
- press "set"
- if you need to adjust the brightness in a daylight conditions, you can use the "up" or "down" buttons to set the "max brightness" percentage
- press "set"

## Using optional Weather syncronization and display

The optional display of weather information requires a Pico W for wireless connection, a configured WiFi connection with SSID and Password, and a no-cost (but valid credit card required) subscription to the OpenWeatherMap.org, "One Call 3.0 API".  It requires modification to the UserConfig.cmake file, and compilation of the source code to create a .uf2 file which is loaded to the Pico W.  Please review the information regarding the "One Call 3.0 API" on the OpenWeatherMap.org website.  The API allows up to 1000 free calls per day, but requires a valid credit card subsciption to pay for the number of calls greater than 1000.  The optional periodic weather updates will use less than 50 calls per day, unless the source code is modified.  The use of the Update Weather Now menu option will also use a "One Call 3.0 API" call.  You have the ability to limit the number of calls per day through the OpenWeatherMap.org website.  By subscribing to the 'One Call 3.0 API", your account will be provided with an API (appid) key which must be configured in the OPEN_WEATHER_MAP_URL field in the UserConfig.cmake file. You will also need to provide the longitude and latitude of the location representing the weather information.

Please review the comments in the UserConfig.cmake file for more information regarding the configuration of the weather option.
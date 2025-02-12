# Used pico board, pico or pico_w
set(PICO_BOARD pico_w)

# Enable/disable debug traces to stdio. Traces can be filtered by file in the 
# isEnabledForFile method of src/Utils/Trace.cpp
set(TRACE_TO_STDIO "0")

# Configuration of the optional Weather menu.  Requires configuration of OPEN_WEATHER_MAP_URL, WIFI_SSID, and WIFI_PASSWORD below
set(INCLUDE_WEATHER "0")  # 0 or 1, 1 = Include the Weather Submenu and processing.  See below for configuration notes.   

set (WIFI_SSID "")
set (WIFI_PASSWORD "")

add_compile_definitions(

    OPEN_WEATHER_MAP_URL=\"\"  # There are so many options in the API, it is easiest to define them as a group. 
#   \"/data/3.0/onecall?lat=11.1111&lon=-22.2222&appid=supersecretkey&exclude=minutely,hourly,daily,alerts&units=imperial\"

    UTC_OFFSET=1 # UTC offset as real number of hours. For example, set 5.5 for UTC+05:30.


    # Set location for automatic daylight saving time observation. 
    # Possible values: Europe, USA or Unknown (no automatic change)
    DST_LOCATION=Unknown

    # This can be enabled to simulate the three buttons using the standard input. Enter triggers SET
    # and the arrow keys trigger UP and DOWN.
    # SIMULATE_BUTTONS_FROM_STDIO
)

# Configuration notes for the Weather option.  
#
# The optional display of weather information requires a Pico W for wireless connection, a configured WiFi connection with SSID
# and Password, and a no-cost (but valid credit card required) subscription to the OpenWeatherMap.org: 'One Call 3.0 API'.
# Please review the information regarding the 'One Call 3.0 API' at OpenWeatherMap.org.  
# By subscribing to the 'One Call 3.0 API', your account will be provided with an API (appid) key which must be configured 
# in the OPEN_WEATHER_MAP_URL field as described in the following example.

# The format of of a working OPEN_WEATHER_MAP_URL looks like the following:
#   \"/data/3.0/onecall?lat=11.1111&lon=-22.2222&appid=supersecretkey&exclude=minutely,hourly,daily,alerts&units=imperial\"

# The url is enclosed by escaped double quotation marks at the beginning and end of the url string: \" 
# A null string is represented by \"\".  If the OPEN_WEATHER_MAP_URL is configured as the null string, weather updates will not be
# attempted, although the weather menu will be displayed if INCLUDE_WEATHER is set to "1".  

# The individual components of the url string are described below:
#
# /data/3.0/onecall?  is a required component of the string, and specifies the use of the 'One Call 3.0 API'.
#
# &lat=11.1111 is a required component of the string, representing the latitude of the location.  Northern Hemisphere values are positive.
#
# &lon=-22.2222 is a requirement component of the string, representing the longitude of the location. Locations to the East of the prime
#     meridian are positive.
#
# &appid=supersecretkey is a required component of the string, and should be replaced by your OpenWeatherMap 'One Call 3.0 API' key.
#     The key should not be enclosed in quotation marks.
#
# &exclude=minutely,hourly,daily,alerts is a required component of the string, and limits the information being returned by the API.
#
# &units=imperial is an optional component of the string, but is highly recommended. Acceptable values are imperial, metric, and 
#     standard.  The default is standard, which will represent temperatures in degrees Kelvin. Please note, that with the exception of
#     windspeed, all values are displayed exactly as they are received from the 'One Call 3.0 API'.  Metric windspeeds are converted
#     from metres/sec to kilometres/hour.  Please review the OpenWeatherMap.org website for further information.

# In addition to the OPEN_WEATHER_MAP_URL, the weather option requires a working Wifi connection. WIFI_SSID and WIFI_PASSWORD must be 
# configured.  For example, to specify your WIFI_SSID as MyWifi, the value would appear as WIFI_SSID="MyWifi" 
# A null string is represented by "". If either the WIFI_SSID or WIFI_PASSWORD are configured as a null string, and INCLUDE_WEATHER is
# set to "1", a compile error will result.    
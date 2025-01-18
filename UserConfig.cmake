# Used pico board, pico or pico_w
set(PICO_BOARD pico_w)

# Enable/disable debug traces to stdio. Traces can be filtered by file in the 
# isEnabledForFile method of src/Utils/Trace.cpp
set(TRACE_TO_STDIO "0")

add_compile_definitions(
    OPEN_WEATHER_APPID=\"\"
    OPEN_WEATHER_UNITS=\"\"
    WIFI_SSID=\"\"
    WIFI_PASSWORD=\"\"
    UTC_OFFSET=0 # UTC offset as real number of hours. For example, set 5.5 for UTC+05:30.

    # Set location for automatic daylight saving time observation. 
    # Possible values: Europe, USA or Unknown (no automatic change)
    DST_LOCATION=Europe

    # This can be enabled to simulate the three buttons using the standard input. Enter triggers SET
    # and the arrow keys trigger UP and DOWN.
    # SIMULATE_BUTTONS_FROM_STDIO
)

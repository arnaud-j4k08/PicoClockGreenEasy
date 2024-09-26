# Used pico board, pico or pico_w
set(PICO_BOARD pico_w)

# Enable/disable debug traces to stdio. Traces can be filtered by file in the 
# isEnabledForFile method of src/Utils/Trace.cpp
set(TRACE_TO_STDIO "0")

add_compile_definitions(
    WIFI_SSID=\"\"
    WIFI_PASSWORD=\"\"
    UTC_OFFSET=1 # UTC offset as real number of hours. For example, set 5.5 for UTC+05:30.

    # Set location for automatic daylight saving time observation. Currently, only "Europe" and
    # "Unknown" (no automatic change) are supported.
    DST_LOCATION=Europe

    SIMULATE_BUTTONS_FROM_STDIO
)

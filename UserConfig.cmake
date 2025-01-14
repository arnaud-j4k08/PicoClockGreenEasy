# Used pico board, pico or pico_w
set(PICO_BOARD pico_w)

# Enable/disable debug traces to stdio. Traces can be filtered by file in the 
# isEnabledForFile method of src/Utils/Trace.cpp
set(TRACE_TO_STDIO "0")

add_compile_definitions(
    WIFI_SSID=\"TowneRidge\"
    WIFI_PASSWORD=\"FredEFarquar\"
    UTC_OFFSET=-5 # UTC offset as real number of hours. For example, set 5.5 for UTC+05:30.

    # Set location for automatic daylight saving time observation. 
    # Possible values: Europe, USA or Unknown (no automatic change)
    DST_LOCATION=USA

    # This can be enabled to simulate the three buttons using the standard input. Enter triggers SET
    # and the arrow keys trigger UP and DOWN.
    # SIMULATE_BUTTONS_FROM_STDIO
)

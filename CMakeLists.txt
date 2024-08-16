include (UserConfig.cmake)

cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(PicoClockGreenEasy)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

set(DISPLAY_PIO "1") # Enable use of DMA and PIO for the display

add_executable( ${PROJECT_NAME}
                src/Bitmap.cpp
                src/Clock.cpp
                src/ClockUi.cpp
                src/main.cpp
                src/fonts.cpp
                src/Settings.cpp
                src/UiTexts.cpp

                src/Functions/AbstractFunction.cpp
                src/Functions/Alarm.cpp
                src/Functions/Countdown.cpp
                src/Functions/Date.cpp
                src/Functions/Options.cpp            
                src/Functions/SkipNextAlarm.cpp
                src/Functions/Stopwatch.cpp
                src/Functions/Submenu.cpp
                src/Functions/Time.cpp
                src/Functions/Temperature.cpp
                src/Functions/WifiStatus.cpp

                src/PicoClockHw/Button.cpp
                src/PicoClockHw/Buzzer.cpp
                src/PicoClockHw/Display.cpp
                src/PicoClockHw/Flash.cpp
                src/PicoClockHw/Platform.cpp
                src/PicoClockHw/Rtc.cpp
)

target_link_libraries(  ${PROJECT_NAME} 
                        pico_stdlib 
                        hardware_i2c 
                        hardware_adc 
                        hardware_pwm)

if (TRACE_TO_STDIO)
        add_compile_definitions(TRACE_TO_STDIO)
        target_sources(${PROJECT_NAME} PRIVATE src/Utils/Trace.cpp)
endif()

if (DISPLAY_PIO)
        add_compile_definitions(DISPLAY_PIO)
        pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/PicoClockHw/Display.pio)
        target_link_libraries(${PROJECT_NAME} hardware_dma hardware_pio)
endif()

if (PICO_CYW43_SUPPORTED) # set by the pico_w.cmake file of the Pico SDK when PICO_BOARD=pico_w
        target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_CYW43_SUPPORTED)
        target_sources(${PROJECT_NAME} PRIVATE
                src/PicoClockHw/Ntp.cpp 
                src/PicoClockHw/Wifi.cpp)
        target_link_libraries(${PROJECT_NAME} pico_cyw43_arch_lwip_threadsafe_background)
endif()

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src )
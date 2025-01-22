# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ===================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")
# Include the user configuration. The UserConfig.cmake at the root of the repo can be overriden by 
# another UserConfig.cmake file one directory upper (outside of the repo to avoid having to modify 
# it).
if (EXISTS "../UserConfig.cmake")
        include (../UserConfig.cmake)
else()
        include (UserConfig.cmake)
endif()

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

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
                src/DaylightSavingTime.cpp
                src/ClockUi.cpp
                src/main.cpp
                src/fonts.cpp
                src/Settings.cpp
                src/UiTexts.cpp


                src/Functions/AbstractFunction.cpp
                src/Functions/Alarm.cpp
                src/Functions/AlarmSubmenu.cpp
                src/Functions/Countdown.cpp
                src/Functions/Date.cpp
                src/Functions/Options.cpp            
                src/Functions/SkipNextAlarm.cpp
                src/Functions/Stopwatch.cpp
                src/Functions/Submenu.cpp
                src/Functions/SyncInfo.cpp
                src/Functions/SyncNow.cpp
                src/Functions/SyncSource.cpp
                src/Functions/Temperature.cpp
                src/Functions/Time.cpp
                src/Functions/WifiStatus.cpp
                src/Functions/WeatherInfo.cpp

                src/PicoClockHw/Button.cpp
                src/PicoClockHw/Buzzer.cpp
                src/PicoClockHw/Display.cpp
                src/PicoClockHw/Flash.cpp
                src/PicoClockHw/Gps.cpp
                src/PicoClockHw/HttpRequest.cpp
                src/PicoClockHw/Platform.cpp
                src/PicoClockHw/Rtc.cpp
)

target_link_libraries(  ${PROJECT_NAME} 
                        pico_stdlib 
                        pico_rand
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
        target_link_libraries(${PROJECT_NAME} 
                pico_cyw43_arch_lwip_threadsafe_background
                pico_lwip_http
                pico_lwip_mbedtls
                pico_mbedtls)

        # Ignore warnings from lwip code
        set_source_files_properties(
                ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
                ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_proxyconnect.c
                PROPERTIES
                COMPILE_OPTIONS "-Wno-unused-result"
                )                
endif()

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# TODO: change this so that the lwip and mbedtls configs do not have to be at the root
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src )
#pragma once

#include <cstdint>

class Platform
{
public:
    static void initStdIo();
    static void enableWatchdog();
    static void runMainLoop();
    static int getCharNonBlocking();
    static uint64_t timeUs();
    static uint32_t randomNumber32();
};
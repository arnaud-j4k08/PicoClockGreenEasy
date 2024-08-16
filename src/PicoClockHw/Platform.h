#pragma once

#include <cstdint>

class Platform
{
public:
    static void initStdIo();
    static void runMainLoop();
    static int getCharNonBlocking();
    static uint64_t timeUs();
};
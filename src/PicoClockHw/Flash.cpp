#include "Flash.h"
#include "Utils/Trace.h"
#include "Display.h"

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <vector>
#include <cstring>
#include <algorithm>

namespace
{
    // Offset at the end of the flash with enough room for one sector
    const uint32_t FLASH_TARGET_OFFSET = PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;

    // Corresponding address
    const uint8_t *g_flashTargetContent = 
        reinterpret_cast<const uint8_t *>(XIP_BASE + FLASH_TARGET_OFFSET);

    // Delay before the modified memory block gets written to flash.
    const uint32_t WRITE_DELAY_MS = 60 * 1000;

    const uint8_t HEADER_VERSION = 1;

    struct Header
    {
        uint8_t version; // Version of the header (not of the data)
        size_t dataSize;
        uint32_t dataHash;
    };

    uint32_t hash(const uint8_t *data, size_t size)
    {
        // Calculate djb2 hash
        uint32_t hash = 5381;
        while (size != 0)
        {
            hash = (hash << 5) + hash + *data;
            data++;
            size--;
        }
        return hash;        
    }
}

uint8_t *Flash::m_data = nullptr;
size_t Flash::m_size = 0;
alarm_id_t Flash::m_writeAlarm = -1;

bool Flash::attach(uint8_t *data, size_t size)
{
    if (sizeof(Header) + size > FLASH_SECTOR_SIZE)
    {
        TRACE << "Total data size bigger than a sector not supported";
        return false;
    }

    m_data = data;
    m_size = size;
    return true;
}

int Flash::read()
{
    if (m_data == nullptr)
    {
        TRACE << "No data attached";
        return -1;
    }

    auto *header = reinterpret_cast<const Header *>(g_flashTargetContent);
    if (header->version != HEADER_VERSION)
    {
        TRACE << "Unsupported header version or unexpected content";
        return -1;
    }
    if (header->dataSize > FLASH_SECTOR_SIZE - sizeof(Header))
    {
        TRACE << "Invalid size";
        return -1;
    }
    if (header->dataHash != hash(g_flashTargetContent + sizeof(Header), header->dataSize))
    {
        TRACE << "Invalid hash";
        return -1;
    }

    // Data present in the flash can be shorter than the attached data block if it was written by an
    // old version of the program. It can also be longer if it was written by a newer version.
    size_t bytesToRead = std::min(header->dataSize, m_size);

    memcpy(m_data, g_flashTargetContent + sizeof(Header), bytesToRead);

    return bytesToRead;
}

void Flash::scheduleWrite()
{
    TRACE << "Data size:" << m_size;
    if (m_data)
    {
        if (m_writeAlarm != -1)
            cancel_alarm(m_writeAlarm);

        m_writeAlarm = add_alarm_in_ms(WRITE_DELAY_MS, &Flash::write, nullptr, false);
    } else
        TRACE << "No data attached";
}

int64_t Flash::write(alarm_id_t id, void *user_data)
{
    if (m_size == reinterpret_cast<const Header *>(g_flashTargetContent)->dataSize &&
        memcmp(m_data, g_flashTargetContent + sizeof(Header), m_size) == 0)
    {
        TRACE << "Data did not change, no need to flash.";
        m_writeAlarm = -1;
        return 0;        
    }

    // Prepare header
    Header header;
    header.version = HEADER_VERSION;
    header.dataSize = m_size;
    header.dataHash = hash(m_data, m_size);

    // Make a copy of the header and data, padded with zeroes and aligned on pages
    std::vector<uint8_t> dataCopy(
        (sizeof(Header) + m_size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE, 0);
    memcpy(dataCopy.data(), &header, sizeof(header));
    memcpy(dataCopy.data() + sizeof(header), m_data, m_size);

#ifndef DISPLAY_PIO
    // As row scanning cannot run during flashing, turn off the display.
    Display *display = Display::instance();
    int savedBrightness = 0;
    if (display != nullptr)
    {
        savedBrightness = display->brightness();
        display->setBrightness(0);
    }
#endif

    // Erase and program
    uint32_t interrupts = save_and_disable_interrupts();
    TRACE <<"Erasing target region...";
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    TRACE <<"Programming target region...";
    flash_range_program(FLASH_TARGET_OFFSET, dataCopy.data(), dataCopy.size());
    restore_interrupts(interrupts);

#ifndef DISPLAY_PIO
    // Turn on display again
    if (display)
        display->setBrightness(savedBrightness);
#endif

    // Do not reschedule
    m_writeAlarm = -1;
    return 0;
}

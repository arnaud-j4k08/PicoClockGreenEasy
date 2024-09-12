#pragma once

#include "Utils/CyclicCounter.h"
#include "Utils/MovingAverage.h"

#include <functional>
#include <pico/time.h>

class Display
{
public:
    static const int WIDTH = 24;
    static const int HEIGHT = 8;
    static const int MATRIX_LEFT = 2;
    static const int MATRIX_TOP = 1;
    static const int MATRIX_WIDTH = 22;
    static const int MATRIX_HEIGHT = 7;

#ifdef DISPLAY_PIO
    // Frame rate is higher if DISPLAY_PIO is enabled, so that the denominator passed to
    // dma_timer_set_fraction can fit into a uint16_t
    static const int FRAME_RATE = 250;
#else
    static const int FRAME_RATE = 1000 / HEIGHT;
#endif

    // The given frameCallback function is called between frame scans to minimize glitches caused
    // by incompletely rendered frame.
    Display(
        const uint32_t *frameBuffer, std::function<void(Display &)> frameCallback = nullptr);

    ~Display();

    static Display *instance()
    {
        return m_instance;
    }

    // Return a value from 0 (dark) to 100 (bright).
    float ambientLight() const; 
    
    void setBrightness(float percent);

private:
    static Display *m_instance;
    const uint32_t *m_frameBuffer;
    std::function<void(Display &)> m_frameCallback;
    mutable int m_ambientLight = 0;
    mutable MovingAverage<256> m_ambientLightFilter {0};

#ifdef DISPLAY_PIO
    void initSendPixelsPioStateMachine();
    void initSelectRowsPioStateMachine();
    void initDma();
    static void onDmaTransferredFrame();

    uint m_sendPixelsSm = 0, m_selectRowsSm = 0;
    int m_dataChannel = -1;
    int m_ctrlChannel = -1;
#else
    bool rowScan();
    
    repeating_timer m_timer;
    CyclicCounter m_currentRow {HEIGHT, 0};
#endif
};

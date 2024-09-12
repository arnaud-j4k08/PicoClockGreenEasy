#include "Display.h"
#include "Utils/Trace.h"
#include "gpio.h"
#include "Utils/Trampoline.h"

#include <hardware/gpio.h>
#include <hardware/adc.h>
#include <hardware/pwm.h>
#include <iostream>

#ifdef DISPLAY_PIO
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>

#include "Display.pio.h"
#endif

namespace 
{
    const int AMBIENT_LIGHT_HYSTERESIS = 1;
    const int PWM_WRAP = 1000;
#ifdef DISPLAY_PIO
    PIO g_pio = pio0;
#endif
}

Display *Display::m_instance = nullptr;

Display::Display(const uint32_t *frameBuffer, std::function<void(Display &)> frameCallback) 
    : m_frameBuffer(frameBuffer), m_frameCallback(frameCallback) 
{
    TRACE << "Display constructor";
    // Configure GPIOs used for sending data to the LED matrix controller
    gpio_init(A0);
    gpio_init(A1);
    gpio_init(A2);
    gpio_init(SDI);
    gpio_init(LE);
    gpio_init(CLK);
    gpio_set_dir(A0, GPIO_OUT);
    gpio_set_dir(A1, GPIO_OUT);
    gpio_set_dir(A2, GPIO_OUT);
    gpio_set_dir(SDI, GPIO_OUT);
    gpio_set_dir(LE, GPIO_OUT);
    gpio_set_dir(CLK, GPIO_OUT);

    // Configure reading of the ambient light sensor
    adc_init();
    adc_gpio_init(AIN);
    adc_select_input(0); // Select ADC input 0, which is ADC_LIGHT
    
    // Configure PWM to adjust display brightness
    gpio_set_function(OE, GPIO_FUNC_PWM);
    int slice = pwm_gpio_to_slice_num(OE);

    // Some leds do not light uniformely if the brightness is low and the PWM frequency is high.
    // Dividing the frequency prevents that.
    pwm_set_clkdiv(slice, 2);

    pwm_set_wrap(slice, PWM_WRAP);

    // Initially set to minimum brightness to avoid a flash on startup (represented by the maximum as OE is 
    // active low)
    pwm_set_gpio_level(OE, PWM_WRAP);

    pwm_set_enabled(slice, true);

    m_instance = this;

#ifdef DISPLAY_PIO
    initSendPixelsPioStateMachine();
    initSelectRowsPioStateMachine();

    // Start both state machines
    pio_set_sm_mask_enabled(g_pio, (1<<m_sendPixelsSm) | (1<<m_selectRowsSm), true);

    initDma();
#else
    MAKE_TRAMPOLINE(Display, rowScan, repeating_timer_t)
    add_repeating_timer_ms(-1, rowScan, this, &m_timer);
#endif

    TRACE << "Display constructor done";
}

Display::~Display()
{
    m_instance = nullptr;

#ifdef DISPLAY_PIO
    // Stop DMA and unclaim channels
    dma_channel_abort(m_dataChannel);
    dma_channel_abort(m_ctrlChannel);
    dma_timer_unclaim(0);
    dma_channel_unclaim(m_dataChannel);
    dma_channel_unclaim(m_ctrlChannel);

    // Stop state machines and unclaim them
    pio_set_sm_mask_enabled(g_pio, (1<<m_sendPixelsSm) | (1<<m_selectRowsSm), false);
    pio_sm_unclaim(g_pio, m_sendPixelsSm);
    pio_sm_unclaim(g_pio, m_selectRowsSm);
#else
    cancel_repeating_timer(&m_timer);
#endif
}

#ifdef DISPLAY_PIO
void Display::initSendPixelsPioStateMachine()
{
    // Claim state machine and configure GPIOs
    m_sendPixelsSm = pio_claim_unused_sm(g_pio, true);
    pio_gpio_init(g_pio, SDI);
    pio_gpio_init(g_pio, CLK);
    pio_gpio_init(g_pio, LE);
    pio_sm_set_consecutive_pindirs(g_pio, m_sendPixelsSm, SDI, 1, true);
    pio_sm_set_consecutive_pindirs(g_pio, m_sendPixelsSm, CLK, 1, true);
    pio_sm_set_consecutive_pindirs(g_pio, m_sendPixelsSm, LE, 1, true);
    
    // Configure state machine
    uint offset = pio_add_program(g_pio, &send_pixels_program);
    pio_sm_config c = send_pixels_program_get_default_config(offset);
    sm_config_set_out_pins(&c, SDI, 1);
    sm_config_set_out_shift(&c, false /* shift OSR to left */, false /* autopull disabled */, 0);
    sm_config_set_sideset_pins(&c, CLK);
    sm_config_set_set_pins(&c, LE, 1);
    pio_sm_init(g_pio, m_sendPixelsSm, offset, &c);
}

void Display::initSelectRowsPioStateMachine()
{
    // Claim state machine and configure GPIOs
    m_selectRowsSm = pio_claim_unused_sm(g_pio, true);
    pio_gpio_init(g_pio, A0);
    pio_gpio_init(g_pio, A1);
    pio_gpio_init(g_pio, A2);
    pio_sm_set_consecutive_pindirs(g_pio, m_selectRowsSm, A0, 1, true); 
    pio_sm_set_consecutive_pindirs(g_pio, m_selectRowsSm, A1, 1, true); 
    pio_sm_set_consecutive_pindirs(g_pio, m_selectRowsSm, A2, 1, true);
    
    // Prepare state machine configuration
    uint offset = pio_add_program(g_pio, &select_rows_program);
    pio_sm_config c = select_rows_program_get_default_config(offset);

    // Assign "set pins" to A0 and A1, with the ignored pin 17 in between
    sm_config_set_set_pins(&c, A0, 3);

    // Side set will control A2
    sm_config_set_sideset_pins(&c, A2);

    // Init SM
    pio_sm_init(g_pio, m_selectRowsSm, offset, &c);
}

void Display::initDma()
{
    // The data channel will transfer the frame buffer to the led matrix controller, whereas the
    // control channel will restart it after every frame.
    m_dataChannel = dma_claim_unused_channel(true);
    m_ctrlChannel = dma_claim_unused_channel(true);

    // Prepare the configuration of the data channel
    dma_channel_config cfg = dma_channel_get_default_config(m_dataChannel);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32); // Each element is a row
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);

    // Configure a DMA timer that will pace the transfer so that the time to display a frame 
    // corresponds to the frame rate.
    dma_timer_claim(0);
    dma_timer_set_fraction(0, 1, clock_get_hz(clk_sys) / FRAME_RATE / HEIGHT); 
    channel_config_set_dreq(&cfg, dma_get_timer_dreq(0));
    
    // Chain the data channel to the control channel, which will continuously restart the data channel.
    channel_config_set_chain_to(&cfg, m_ctrlChannel);

    dma_channel_configure(
        m_dataChannel, 
        &cfg, 
        &pio0_hw->txf[0],   // Write to the TX FIFO of the "send pixels" SM
        m_frameBuffer, 
        HEIGHT, 
        false               // do no start immediately
    );

    // Install the IRQ handler that will be called whenever one frame transfer is completed.
    dma_channel_set_irq0_enabled(m_dataChannel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, onDmaTransferredFrame);
    irq_set_enabled(DMA_IRQ_0, true);

    // Configure the control channel to reset the data channel read address and retrigger it.
    cfg = dma_channel_get_default_config(m_ctrlChannel);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, false);
    dma_channel_configure(
        m_ctrlChannel, 
        &cfg, 
        &dma_hw->ch[m_dataChannel].al3_read_addr_trig,  // Write data channel read address and trigger
        &m_frameBuffer,                                 // Read address of frame buffer
        1,                                              // This address is the only element
        false                                           // Do no start immediately
    );

    // Now that everything is ready, start the continuous transfer. The led matrix controller is fully
    // driven by DMA and PIO, so that the CPU does not have to handle anything for it.
    dma_channel_start(m_dataChannel);
}

void Display::onDmaTransferredFrame()
{
    if (m_instance->m_frameCallback)
        m_instance->m_frameCallback(*m_instance);

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << m_instance->m_dataChannel;
}

#else // DISPLAY_PIO

bool Display::rowScan()
{
    // Send all pixels for the current row.
    uint32_t rowBits = m_frameBuffer[m_currentRow];
    for (int i = 0; i < 32; i++)
    {
        gpio_put(CLK, false);
        gpio_put(SDI, rowBits & (1<<31));
        rowBits <<= 1;
        gpio_put(CLK, true);
    }
    
    // Latch to the matrix controler
    gpio_put(LE, true);
    gpio_put(LE, false);

    // Select row
    gpio_put(A0, m_currentRow & 1);
    gpio_put(A1, m_currentRow & 2);
    gpio_put(A2, m_currentRow & 4);

    // Update row counter
    if (m_currentRow.increment())
    {
        if (m_frameCallback)
            m_frameCallback(*this);
    }

    return true; // to continue repeating
}
#endif // DISPLAY_PIO

float Display::ambientLight() const
{
    uint16_t adc = 4095 - adc_read();

    // Stabilize the output of the DAC using a filter
    m_ambientLightFilter.put(adc * 100.0f / 4095);
    return m_ambientLightFilter.get();
}

void Display::setBrightness(float percent)
{
    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;

    TRACE << "Set brightness to" << percent << "%";

    // OE is active low, so reverse the percentage.
    // Also set a level of at least 1 so that the display does not completely turn off.
    pwm_set_gpio_level(OE, PWM_WRAP - std::max(1.0f, percent * PWM_WRAP / 100));
}
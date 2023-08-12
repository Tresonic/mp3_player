#include <stdio.h>

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "i2s_dac.pio.h"
#include "config.h"
#include "player.h"

static int16_t zerobuf[1024];
static int dma_channel;
static uint lastDma = 0;


void __isr __time_critical_func(dma_handler)() {
    dma_hw->ints0 = 1u << dma_channel;
    int16_t* block = audiobuffer.getNextDmaBlock();
    uint now = time_us_32();
    // printf("dma! ms since last: %i\n", (int)(now-lastDma));
    lastDma = now;
    if (block) {
        dma_channel_transfer_from_buffer_now(dma_channel, block, config::AUDIOBUFFER_SIZE / 2);
    } else {
        puts("zero dma");
        dma_channel_transfer_from_buffer_now(dma_channel, zerobuf, sizeof(zerobuf) / 2);
    }

    // puts("dma started");
}

void set_pio_frequency(unsigned int sample_freq) {
    // printf("setting pio freq %d\n", (int) sample_freq);
    // uint32_t system_clock_frequency = clock_get_hz(clk_sys);
    // assert(system_clock_frequency < 0x40000000);
    // uint32_t divider = system_clock_frequency * 4 / sample_freq; // avoid arithmetic overflow
    // printf("System clock at %u, I2S clock divider 0x%x/256\n", (uint) system_clock_frequency, (uint)divider);
    // assert(divider < 0x1000000);
    // pio_sm_set_clkdiv_int_frac(pio0, 0, divider >> 8u, divider & 0xffu);

    float bitClk = sample_freq * 16 /* bits per sample */ * 2.0 /* channels */ * 2.0 /* edges per clock */;
    pio_sm_set_clkdiv(pio0, 0, (float)clock_get_hz(clk_sys) / bitClk);
}

void init_pio(int pin_clk, int pin_data) {
    gpio_set_function(pin_clk, GPIO_FUNC_PIO0);
    gpio_set_function(pin_clk + 1, GPIO_FUNC_PIO0);
    gpio_set_function(pin_data, GPIO_FUNC_PIO0);

    // Choose PIO instance (0 or 1)
    PIO pio = pio0;

    // Get first free state machine in PIO 0
    uint sm = pio_claim_unused_sm(pio, true);

    // Add PIO program to PIO instruction memory. SDK will find location and
    // return with the memory offset of the program.
    uint offset = pio_add_program(pio, &i2s_dac_program);


    // Initialize the program using the helper function in our .pio file
    i2s_dac_program_init(pio, sm, offset, pin_data, pin_clk);

    set_pio_frequency(48000);

    // not_ Start running our PIO program in the state machine
    pio_sm_set_enabled(pio, sm, false);
    puts("pio not enabled");

    dma_channel = dma_claim_unused_channel(true);
    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_dreq(&dma_config, DREQ_PIO0_TX0);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_high_priority(&dma_config, true);
    dma_channel_configure(dma_channel,
                          &dma_config,
                          &pio->txf[sm], //dest is sm 0
                          NULL, //src
                          0, //count
                          false //trigger
    );
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_priority(DMA_IRQ_0, 0);
    puts("dma handler set");
    
    dma_channel_set_irq0_enabled(dma_channel, true);
    puts("marker");
}

void i2s_dac_set_enabled(bool enabled) {
    irq_set_enabled(DMA_IRQ_0, enabled);
    if (enabled) {
        dma_handler();
    }
    pio_sm_set_enabled(pio0, 0, enabled);
}
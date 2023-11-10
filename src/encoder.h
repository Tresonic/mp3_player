#include <stdio.h>

#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "config.h"
#include "encoder.pio.h"

static PIO pio;
static uint sm;
static int rotation;

static int get_rotation() {
    static int rotation_old;
    int ret = 0;
    if (rotation>rotation_old) {
        ret = 1;
    } else if (rotation<rotation_old) {
        ret = -1;
    }
    rotation_old = rotation;
    return ret;
}

static void pio_irq_handler()
{
    // test if irq 0 was raised
    if (pio1_hw->irq & 1) {
        rotation++;
    }
    // test if irq 1 was raised
    if (pio1_hw->irq & 2) {
        rotation--;
    }
    // clear both interrupts
    pio1_hw->irq = 3;
}

void init_encoder_pio(int pin_A, int pin_B)
{
    PIO pio = pio1;

    uint sm = pio_claim_unused_sm(pio, true);

    uint offset = pio_add_program(pio, &encoder_program);

    pio_sm_config conf = encoder_program_get_default_config(offset);

    // configure the used pins as input with pull up
    pio_gpio_init(pio, pin_A);
    gpio_set_pulls(pin_A, true, false);
    pio_gpio_init(pio, pin_B);
    gpio_set_pulls(pin_B, true, false);

    // set the 'in' pins
    sm_config_set_in_pins(&conf, pin_A);
    // set shift to left: bits shifted by 'in' enter at the least
    // significant bit (LSB), no autopush
    sm_config_set_in_shift(&conf, false, false, 0);
    // set the IRQ handler
    irq_set_exclusive_handler(PIO1_IRQ_0, pio_irq_handler);
    // enable the IRQ
    irq_set_enabled(PIO1_IRQ_0, true);
    pio1_hw->inte0 = PIO_IRQ0_INTE_SM0_BITS | PIO_IRQ0_INTE_SM1_BITS;
    // init the sm.
    // Note: the program starts after the jump table -> initial_pc = 16
    pio_sm_init(pio, sm, 16, &conf);
    // enable the sm
    pio_sm_set_enabled(pio, sm, true);
}

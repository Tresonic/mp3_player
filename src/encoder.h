#include <stdio.h>

#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "config.h"
#include "encoder.pio.h"


static volatile int rotation;

uint8_t lrmem = 3;
int lrsum = 0;

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

static void rotary_irq(unsigned int gpio, uint32_t mask) {
    static int8_t TRANS[] = {0,  -1, 1, 14, 1,  0, 14, -1,
                             -1, 14, 0, 1,  14, 1, -1, 0};
    int8_t l, r;


    l = gpio_get(config::PIN_ENCODER_A);
    r = gpio_get(config::PIN_ENCODER_B);
    // printf("%i, %i\n", l, r);

    lrmem = ((lrmem & 0x03) << 2) + 2 * l + r;
    lrsum = lrsum + TRANS[lrmem];
    /* encoder not in the neutral state */
    if (lrsum % 4 != 0)
        return;
    /* encoder in the neutral state */
    if (lrsum == 4) {
        lrsum = 0;
        rotation += 1;
        return;
    }
    if (lrsum == -4) {
        lrsum = 0;
        rotation -= 1;
        return;
    }
    /* lrsum > 0 if the impossible transition */
    lrsum = 0;
}

void init_encoder()
{
    // GPIO Setup for Encoder
    // gpio_init(ENC_SW); // Initialise a GPIO for (enabled I/O and set func to
    //                    // GPIO_FUNC_SIO)
    // gpio_set_dir(ENC_SW, GPIO_IN);
    // gpio_disable_pulls(ENC_SW);

    gpio_init(config::PIN_ENCODER_A);
    gpio_set_dir(config::PIN_ENCODER_A, GPIO_IN);
    gpio_pull_up(config::PIN_ENCODER_A);

    gpio_init(config::PIN_ENCODER_B);
    gpio_set_dir(config::PIN_ENCODER_B, GPIO_IN);
    gpio_pull_up(config::PIN_ENCODER_B);

    // gpio_set_irq_enabled_with_callback(ENC_SW, GPIO_IRQ_EDGE_FALL, true,
    //                                    &rotary);
    gpio_set_irq_enabled_with_callback(config::PIN_ENCODER_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &rotary_irq);
    gpio_set_irq_enabled_with_callback(config::PIN_ENCODER_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &rotary_irq);
}

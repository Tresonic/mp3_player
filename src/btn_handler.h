#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "config.h"
#include "encoder.h"
#include "player.h"

void init_btn_handler(uint8_t vol)
{
    init_encoder_pio(config::PIN_ENCODER_A, config::PIN_ENCODER_B);
    rotation = vol;

    gpio_init(config::PIN_BTN_PAUSE);
    gpio_set_pulls(config::PIN_BTN_PAUSE, true, false);
}

void update_btns()
{
    /* Encoder */
    if (rotation < 0)
        rotation = 0;
    else if (rotation > 255)
        rotation = 255;
    else
        player::setVol(rotation);

    /* Buttons */
    if (gpio_get(config::PIN_BTN_PAUSE) == 0) {
        player::togglePause();
        /* TODO actual debounce function */
        sleep_ms(150);
    }
}

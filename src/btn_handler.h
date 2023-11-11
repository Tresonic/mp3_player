#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "config.h"
#include "encoder.h"
#include "player.h"
#include "utils.h"

void init_btn_handler(uint8_t vol) {
    init_encoder();

    gpio_init(config::PIN_BTN_PAUSE);
    gpio_set_pulls(config::PIN_BTN_PAUSE, true, false);
}

void update_btns() {
    int rot = get_rotation();
    if (rot) {
        player::setVol(
            clamp(player::getVol() + rot * config::VOLUME_STEP, 0, UINT8_MAX));
    }

    /* Buttons */
    if (gpio_get(config::PIN_BTN_PAUSE) == 0) {
        player::togglePause();
        /* TODO actual debounce function */
        sleep_ms(150);
    }
}

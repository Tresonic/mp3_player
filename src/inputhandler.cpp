#include "inputhandler.h"

#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "config.h"
#include "encoder.h"
#include "pins.h"
#include "player.h"
#include "utils.h"


namespace inputhandler {
void init() {
    init_encoder();

    gpio_init(config::PIN_BTN_A);
    gpio_set_pulls(config::PIN_BTN_A, true, false);
}

int get_rot() {
    return get_rotation();
}

Buttonpress get_btn_a() {
    static unsigned int last = 0;
    unsigned int now = time_us_32();
    
    static bool old = true;
    if (!gpio_get(config::PIN_BTN_A) && old && now - last > 300000) {
        old = true;
        last = now;
        return Buttonpress::Short;
    }
    return Buttonpress::None;
}
}
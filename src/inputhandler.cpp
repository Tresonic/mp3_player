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

int get_rot() { return get_rotation(); }

Buttonpress get_btn_a() {
    unsigned int now = time_us_32();

    static bool oldIt = true;
    static unsigned int last = 0;
    bool thisIt = gpio_get(config::PIN_BTN_A);

    if (!thisIt && oldIt) {
        last = now;
    } else if (thisIt && !oldIt && now - last > 10000) {
        if (now - last < 750000) {
            puts("shortpress");
            last = now;
            return Buttonpress::Short;
        } else {
            puts("longpress");
            last = now;
            return Buttonpress::Long;
        }
    }
    oldIt = thisIt;

    return Buttonpress::None;
}
} // namespace inputhandler
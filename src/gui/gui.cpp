#include "gui.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hardware/timer.h"
#include "filesystem.h"
#include "display.h"
#include "config.h"
#include "pico/types.h"
#include "inputhandler.h"

enum GuiState {List, Play};
static GuiState state = List;

namespace gui {
    void init() {
        display::init(config::PIN_I2C_CLK, config::PIN_I2C_DATA);
        inputhandler::init();

        gui::filesystem::init();

        while (true) {
            tick();
        }
    }

    void tick() {
        static unsigned int last = 0;
        unsigned int now = time_us_32();

        if (now-last > 100000u) {
            switch (state) {
                case List:
                    gui::filesystem::tick();
                    break;
                case Play:
                    break;
            }
        }
    }
}

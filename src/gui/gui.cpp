#include "gui.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hardware/timer.h"
#include "filesystem.h"
#include "play.h"
#include "display.h"
#include "config.h"
#include "pico/types.h"
#include "inputhandler.h"

namespace gui {
    GuiState state = Play;
    bool dirty = false;

    void init() {
        display::init(config::PIN_I2C_CLK, config::PIN_I2C_DATA);
        inputhandler::init();

        // TODO rename filesystem and/or List
        gui::filesystem::init();
        gui::play::init();

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
                    if (dirty)
                        gui::filesystem::update();
                    gui::filesystem::tick();
                    break;
                case Play:
                    if (dirty)
                        gui::play::update();
                    gui::play::tick();
                    break;
            }
        }
    }

    void setState(GuiState newState) {
        state = newState;
        // call update() on new gui screen
        dirty = true;
    }
}

#include "play.h"

#include "inputhandler.h"
#include "pico/types.h"
#include "pico/stdlib.h"
#include "display.h"
#include "gui.h"
#include "player.h"

namespace gui::play {
    bool dirty = true;

    void init() {

    }

    void update() {
        dirty = true;
    }

    void tick() {

        int rot = inputhandler::get_rot();
        if (rot > 0) {
            player::setVol(player::getVol() + 1);
        } else if (rot < 0) {
            player::setVol(player::getVol() - 1);
        }

        Buttonpress btn_a = inputhandler::get_btn_a();
        if(btn_a == Buttonpress::Short) {
            dirty = true;
            player::togglePause();
        } else if(btn_a == Buttonpress::Long) {
            gui::setState(gui::List);
        }

        if (dirty) {
            dirty = false;

            if (player::isPlaying()) {
                display::print(6, 0, "playing");
            } else {
                display::print(6, 0, "paused");
            }
            display::display();
        }
    }
}

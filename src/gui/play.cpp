#include "play.h"

#include "display.h"
#include "gui.h"
#include "inputhandler.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "player.h"

namespace gui::play {
bool dirty = true;

void init() {}

void update() { dirty = true; }

void printPlayPause(bool is_playing) {

    // uint8_t play[5] = {0xFF, 0x81, 0x42, 0x24, 0x18};
    // uint8_t pause[5] = {0x24, 0x24, 0x24, 0x24, 0x24};
    // uint8_t circle[2][13] = {
    //     {0xC0, 0x20, 0x18, 0x04, 0x02, 0x02, 0x01, 0x02, 0x02, 0x04, 0x18,
    //     0x20,
    //      0xC0},
    //     {0x00, 0x01, 0x06, 0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08, 0x06,
    //     0x01,
    //      0x00},
    // };

    uint8_t playCircle[2][13] = {
        {0xC0, 0x20, 0x18, 0x04, 0x02, 0xFA, 0x09, 0x12, 0x22, 0xC4, 0x18, 0x20,
         0xC0},
        {0x00, 0x01, 0x06, 0x08, 0x10, 0x17, 0x24, 0x12, 0x11, 0x08, 0x06, 0x01,
         0x00},
    };
    uint8_t pauseCircle[2][13] = {
        {0xC0, 0x20, 0x18, 0x04, 0x22, 0x22, 0x21, 0x22, 0x22, 0x04, 0x18, 0x20,
         0xC0},
        {0x00, 0x01, 0x06, 0x08, 0x09, 0x11, 0x21, 0x11, 0x09, 0x08, 0x06, 0x01,
         0x00},
    };

    // display::printCustom(6 * 10 + 1, 6, play, 5);
    // display::printCustom(6 * 10, 6, pause, 5);

    if (is_playing) {
        display::printCustom(6 * 9 + 2, 6, pauseCircle[0], 13);
        display::printCustom(6 * 9 + 2, 6 + 1, pauseCircle[1], 13);
    } else {
        display::printCustom(6 * 9 + 2, 6, playCircle[0], 13);
        display::printCustom(6 * 9 + 2, 6 + 1, playCircle[1], 13);
    }
}

void tick() {

    int rot = inputhandler::get_rot();
    if (rot > 0) {
        player::setVol(player::getVol() + 1);
    } else if (rot < 0) {
        player::setVol(player::getVol() - 1);
    }

    Buttonpress btn_a = inputhandler::get_btn_a();
    if (btn_a == Buttonpress::Short) {
        dirty = true;
        player::togglePause();
    } else if (btn_a == Buttonpress::Long) {
        gui::setState(gui::List);
    }

    if (dirty) {
        dirty = false;

        printPlayPause(player::isPlaying());

        display::display();
    }
}
} // namespace gui::play

#include "play.h"

#include "config.h"
#include "display.h"
#include "gui.h"
#include "inputhandler.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "player.h"
#include <cstdio>
#include <cstring>

namespace gui::play {
bool dirty = true;
bool newFile = true;
char fileTitle[config::MAX_FILE_PATH_LEN] = {0};
char fileLength[6] = {0}; // 6 is enough for MM:SS\0 or HH:SS\0

void init() {}

void update() { dirty = true; }

void updateFile() {
    // TODO theoretically: the file length should only change on a new file
    // This doesn't yet work for files using variable Bitrate

    dirty = true;
    newFile = true;
}

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

void file2title(const char *filePath) {
    char *fileName = strchr(filePath, '/');
    if (fileName) {
        strncpy(fileTitle, fileName + 1, config::MAX_FILE_PATH_LEN);
    } else {
        // This should not be possible
        strncpy(fileTitle, filePath, config::MAX_FILE_PATH_LEN);
    }

    // remove extension
    char *extension = strchr(fileTitle, '.');
    if (extension)
        *extension = '\0';
}

void length2string(unsigned length) {
    unsigned hour = 0, min, sec;

    min = length / 60;
    sec = length - min * 60;
    if (min >= 60) {
        hour = min / 60;
        min -= hour * 60;
        if (hour > 99)
            // this will probably just happen on errors
            // 3 digit hours would overflow fileLength
            sprintf(fileLength, "00:00");
        else
            sprintf(fileLength, "%02d:%02d", hour, min);
    } else {
        sprintf(fileLength, "%02d:%02d", min, sec);
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

    if (newFile) {
        newFile = false;
        file2title(player::getFile());
        length2string(player::getLength());
    }

    if (dirty) {
        dirty = false;

        // 8 is the fonts height
        display::printCentered(8, fileTitle);
        display::printCentered(4 * 8, fileLength);

        printPlayPause(player::isPlaying());

        display::display();
    }
}
} // namespace gui::play

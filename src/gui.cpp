#include "gui.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hardware/timer.h"
#include "inputhandler.h"
#include "display.h"
#include "config.h"
#include "filemanager.h"
#include "inputhandler.h"
#include "pico/types.h"
#include "player.h"

enum GuiState {List, Play};
static GuiState state = List;
bool dirty = true;
bool newDir = true;
uint listIdx = 0;
char** fileList;
char** dirList;
char* currentDir;


namespace gui {
void init() {
    display::init(config::PIN_I2C_CLK, config::PIN_I2C_DATA);
    fileList = (char**)malloc(8 * sizeof(char*));
    dirList = (char**)malloc(8 * sizeof(char*));
    for (int i=0; i<8; ++i) {
        fileList[i] = (char*)malloc(config::MAX_FILE_PATH_LEN);
        dirList[i] = (char*)malloc(config::MAX_FILE_PATH_LEN);
        fileList[i][0] = dirList[i][0] = 0;
    }
    currentDir = (char*)malloc(config::MAX_FILE_PATH_LEN);
    strcpy(currentDir, "/");

    inputhandler::init();
    while (true) {
        tick();
    }
}

void tick() {
    static unsigned int last = 0;
    unsigned int now = time_us_32();

    int rot;
    switch (state) {
    case List:
        // puts("list");
        if (newDir) {
            newDir = false;
            listIdx = 0;
            filemanager::list_dir("/", fileList, 8, dirList, 8);
            puts("filedone");
        }

        rot = inputhandler::get_rot();
        if (rot) {
            listIdx += rot;
            dirty = true;
            listIdx %= 8;
        }   

        if(inputhandler::get_btn_a() == Buttonpress::Short) {
            player::play(fileList[listIdx]);
        }

        if (dirty && now-last > 100000u) {
            last = now;
            dirty = false;
            puts("dirty");
            for (int l=0; l<8; ++l) {
                if (fileList[l][0] == 0) break;
                display::print(6, l*8, fileList[l]);
                puts(fileList[l]);
            }
            display::printChar(0, listIdx % 8 * 8, '>');
            display::display();
        }
    break;
    case Play:
    break;
    }
}
}
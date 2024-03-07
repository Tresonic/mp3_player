#include "filesystem.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "display.h"
#include "inputhandler.h"
#include <cstdlib>
#include "pico/types.h"
#include "filemanager.h"
#include "player.h"

namespace gui::filesystem {

    bool dirty = true;
    bool newDir = true;
    int listIdx = 0;
    char** fileList;
    int fileListLen;
    char** dirList;
    int dirListLen;
    char* currentDir;
    char filePath[config::MAX_FILE_PATH_LEN] = { 0 };
    char *BACK_STR = "\x18";

    void init() {

        fileList = (char**)malloc(config::MAX_FILES_IN_LIST * sizeof(char*));
        dirList = (char**)malloc(config::MAX_FILES_IN_LIST * sizeof(char*));

        fileListLen = 0;
        dirListLen = 1;

        for (int i=0; i<8; ++i) {
            fileList[i] = (char*)malloc(config::MAX_FILE_PATH_LEN);
            dirList[i] = (char*)malloc(config::MAX_FILE_PATH_LEN);
            fileList[i][0] = dirList[i][0] = 0;
        }
        currentDir = (char*)malloc(config::MAX_FILE_PATH_LEN);
        strcpy(currentDir, "/");
    }

    void tick() {
        int isSubDir = strcmp(currentDir, "/") != 0;
        // TODO implement scroll (remove %8)
        int maxListIdx = (dirListLen + fileListLen + isSubDir) % 8;

        if (newDir) {
            newDir = false;
            listIdx = 0;
            filemanager::list_dir(currentDir, fileList, fileListLen, dirList, dirListLen);
            dirty = true;
        }

        int rot = inputhandler::get_rot();
        if (rot) {
            listIdx += rot;

            // keep cursor in bounds
            if (listIdx < 0) {
                listIdx = maxListIdx - 1;
            } else if (listIdx >= maxListIdx) {
                listIdx = 0;
            }

            // TODO scroll
            dirty = true;
        }

        if(inputhandler::get_btn_a() == Buttonpress::Short) {
            if (listIdx == 0 && isSubDir) {
                // is currently a subdirectory -> first index is back button -> remove last dir
                for (int i = strlen(currentDir) - 2; i >= 0; i--) {
                    if (currentDir[i] == '/') {
                        currentDir[i + 1] = '\0';
                    }
                }
                newDir = true;
            } else if (listIdx < dirListLen) {
                // TODO strncpy
                // substract indexes that might have come before
                strcat(currentDir, dirList[listIdx - isSubDir]);
                strcat(currentDir, "/");
                newDir = true;
            // TODO check if playlist
            } else {
                // TODO strncpy
                strcpy(filePath, currentDir);
                // substract indexes that might have come before
                strcat(filePath, fileList[listIdx - dirListLen - isSubDir]);
                player::play(filePath);
            }
        }

        if (dirty) {
            dirty = false;
            int display_idx = 0;
            if (strcmp(currentDir, "/")) {
                // is subdirectory
                display::print(6, display_idx*8, BACK_STR);
                display_idx++;
            }
            for (int i = 0; i < dirListLen; i++) {
                display::print(6, display_idx*8, dirList[i]);
                display_idx++;
            }
            for (int i = 0; i < fileListLen; i++) {
                display::print(6, display_idx*8, fileList[i]);
                display_idx++;
            }
            display::printChar(0, listIdx % 8 * 8, '>');
            display::display();
        }
    }
}

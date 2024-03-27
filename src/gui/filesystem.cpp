#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "display.h"
#include "filemanager.h"
#include "inputhandler.h"
#include "pico/types.h"
#include "player.h"
#include "playlistfile.h"
#include "queue.h"

#include <cstdlib>
#include <ctype.h>

namespace gui::filesystem {

bool dirty = true;
bool newDir = true;
int listIdx = 0;
char **fileList;
int fileListLen;
char **dirList;
int dirListLen;
char *currentDir;
char filePath[config::MAX_FILE_PATH_LEN] = {0};
char *BACK_STR = "\x18";

void init() {

    fileList = (char **)malloc(config::MAX_FILES_IN_LIST * sizeof(char *));
    dirList = (char **)malloc(config::MAX_FILES_IN_LIST * sizeof(char *));

    fileListLen = 0;
    dirListLen = 1;

    for (int i = 0; i < 8; ++i) {
        fileList[i] = (char *)malloc(config::MAX_FILE_PATH_LEN);
        dirList[i] = (char *)malloc(config::MAX_FILE_PATH_LEN);
        fileList[i][0] = dirList[i][0] = 0;
    }
    currentDir = (char *)malloc(config::MAX_FILE_PATH_LEN);
    strcpy(currentDir, "/");
}

enum EXTENSION { UNKNOWN, MP3, M3U };

enum EXTENSION getExtension(char *str) {
    // 4 characters without '.' and '\0' should be enough
    size_t maxExtLen = 4;

    char *tmp = strrchr(str, '.');
    if (tmp == NULL) {
        // '.' not found
        return EXTENSION::UNKNOWN;
    }
    char ext[maxExtLen];
    strncpy(ext, tmp + 1, maxExtLen);

    // to lower
    for (size_t i = 0; i < maxExtLen; i++) {
        ext[i] = tolower(ext[i]);
    }

    if (!strncmp(ext, "mp3", 3)) {
        return EXTENSION::MP3;
    } else if (!strncmp(ext, "m3u", 3)) {
        return EXTENSION::M3U;
    } else {
        return EXTENSION::UNKNOWN;
    }
}

void tick() {
    int isSubDir = strcmp(currentDir, "/") != 0;
    // TODO implement scroll (remove %8)
    int maxListIdx = (dirListLen + fileListLen + isSubDir) % 8;

    if (newDir) {
        newDir = false;
        listIdx = 0;
        filemanager::list_dir(currentDir, fileList, fileListLen, dirList,
                              dirListLen);
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

    if (inputhandler::get_btn_a() == Buttonpress::Short) {
        if (listIdx == 0 && isSubDir) {
            // is currently a subdirectory -> first index is back button ->
            // remove last dir
            for (int i = strlen(currentDir) - 2; i >= 0; i--) {
                if (currentDir[i] == '/') {
                    currentDir[i + 1] = '\0';
                }
            }
            newDir = true;
        } else if (listIdx < dirListLen) { // selected is directory
            // substract indexes that might have come before
            strncat(currentDir, dirList[listIdx - isSubDir],
                    config::MAX_FILE_PATH_LEN);
            strncat(currentDir, "/", config::MAX_FILE_PATH_LEN);
            newDir = true;
        } else {
            strncpy(filePath, currentDir, config::MAX_FILE_PATH_LEN);
            // substract indexes that might have come before
            strncat(filePath, fileList[listIdx - dirListLen - isSubDir],
                    config::MAX_FILE_PATH_LEN);

            EXTENSION ext = getExtension(filePath);

            // TODO actions should be changed when more buttons are avaiable
            if (ext == EXTENSION::MP3) { // selected is mp3 file
                queue::add_to_queue_end(filePath);
            } else if (ext ==
                       EXTENSION::M3U) { // selected is playlist(m3u) file
                playlistfile::add_to_queue(filePath);
            }
        }
    }

    if (dirty) {
        dirty = false;
        int display_idx = 0;
        if (strcmp(currentDir, "/")) {
            // is subdirectory
            display::print(6, display_idx * 8, BACK_STR);
            display_idx++;
        }
        for (int i = 0; i < dirListLen; i++) {
            display::print(6, display_idx * 8, dirList[i]);
            display_idx++;
        }
        for (int i = 0; i < fileListLen; i++) {
            display::print(6, display_idx * 8, fileList[i]);
            display_idx++;
        }
        display::printChar(0, listIdx % 8 * 8, '>');
        display::display();
    }
}
} // namespace gui::filesystem

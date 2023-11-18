#include "filemanager.h"

#include <stdio.h>
#include <string.h>

#include "f_util.h"
#include "ff.h"
#include "hw_config.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "rtc.h"
#include "sd_card.h"

#include "config.h"

namespace filemanager {

using config::MAX_OPEN_FILES;

static sd_card_t *mSD;
static FIL mFiles[MAX_OPEN_FILES];
static bool mOpenFiles[MAX_OPEN_FILES];

RET_TYPE list_dir(const char *path, char *files, int files_len, char *dirs,
                  int dirs_len) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int files_index = 0;
    int dirs_index = 0;

    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        while (true) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;

            int len = strlen(fno.fname);
            if (fno.fattrib & AM_DIR) {
                if (dirs_index + len < dirs_len) {
                    strcpy(&dirs[dirs_index], fno.fname);
                    dirs_index += len;
                    dirs[dirs_index++] = '\n';
                }
            } else {
                if (files_index + len < files_len) {
                    strcpy(&files[files_index], fno.fname);
                    files_index += len;
                    files[files_index++] = '\n';
                }
            }
        }
        dirs[dirs_index] = '\0';
        files[files_index] = '\0';
        f_closedir(&dir);
        return RET_SUCCESS;
    } else {
        return RET_ERROR;
    }
}

void initSd() {
    time_init();

    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html
    mSD = sd_get_by_num(0);

    FRESULT fr = f_mount(&mSD->fatfs, mSD->pcName, 1);
    if (FR_OK != fr)
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
}

void deinitSd() { f_unmount(mSD->pcName); }

int openFile(const char *path) {
    for (int i = 0; i < MAX_OPEN_FILES; ++i) {
        if (!mOpenFiles[i]) {
            FRESULT fr = f_open(&mFiles[i], path, FA_READ);
            if (fr) {
                return -1;
            }
            mOpenFiles[i] = true;
            return i;
        }
    }
    // file open limit reached
    return -1;
}

void closeFile(int handle) {
    if (handle < 0 || handle >= MAX_OPEN_FILES) {
        return;
    }
    f_close(&mFiles[handle]);
    mOpenFiles[handle] = false;
}

unsigned int readFileToBuffer(int handle, void *buffer, int numBytes) {
    if (handle < 0 || handle >= MAX_OPEN_FILES || !mOpenFiles[handle]) {
        return -1;
    }

    uint bytesRead;
    f_read(&mFiles[handle], buffer, numBytes, &bytesRead);

    return bytesRead;
}

int seek(int handle, unsigned long pos) {
    if (handle < 0 || handle >= MAX_OPEN_FILES || !mOpenFiles[handle]) {
        return -1;
    }
    return f_lseek(&mFiles[handle], pos);
}

bool eof(int handle) {
    if (handle < 0 || handle >= MAX_OPEN_FILES || !mOpenFiles[handle]) {
        return false;
    }
    return f_eof(&mFiles[handle]);
}

unsigned long long size(int handle) {
    if (handle < 0 || handle >= MAX_OPEN_FILES || !mOpenFiles[handle]) {
        return -1;
    }
    return f_size(&mFiles[handle]);
}

} // namespace filemanager

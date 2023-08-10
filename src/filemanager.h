#pragma once

#include "f_util.h"
#include "ff.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "rtc.h"

#include "config.h"
#include "hw_config.h"

using config::MAX_OPEN_FILES;

int _handle, _numBytes;
uint _bytesRead;
void* _buffer;
FIL* _file;
sd_card_t* mSD;

void core1FileRead() {
    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html
    mSD = sd_get_by_num(0);

    FRESULT fr = f_mount(&mSD->fatfs, mSD->pcName, 1);
    if (FR_OK != fr)
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);

    while (true) {
        multicore_fifo_pop_blocking();
        f_read(_file, _buffer, _numBytes, &_bytesRead);
        multicore_fifo_push_blocking(2);
    }
}

class Filemanager {
public:
    void initSd()
    {
        // // See FatFs - Generic FAT Filesystem Module, "Application Interface",
        // // http://elm-chan.org/fsw/ff/00index_e.html
        // mSD = sd_get_by_num(0);

        // FRESULT fr = f_mount(&mSD->fatfs, mSD->pcName, 1);
        // if (FR_OK != fr)
        //     panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);

        multicore_launch_core1(core1FileRead);
        puts("mounted! and core started");
    }

    void deinitSd() {
        f_unmount(mSD->pcName);
    }

    int openFile(const char* path) {
        for (int i=0; i<MAX_OPEN_FILES; ++i) {
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

    uint readFileToBuffer(int handle, void* buffer, int numBytes) {
        if (handle < 0 || handle >= MAX_OPEN_FILES || !mOpenFiles[handle]) {
            return -1;
        }

        // uint bytesRead;
        // f_read(&mFiles[handle], buffer, numBytes, &bytesRead);
        _handle = handle; _buffer = buffer; _numBytes = numBytes; _file = &mFiles[handle];
        multicore_fifo_push_blocking(1);
        multicore_fifo_pop_blocking();


        return _bytesRead;
    }

    int readDirectoryToBuffer() {
        //TODO
        return 0;
    }

private:

    FIL mFiles[MAX_OPEN_FILES];
    bool mOpenFiles[MAX_OPEN_FILES];
};

Filemanager filemanager;
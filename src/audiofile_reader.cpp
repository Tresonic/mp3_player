#include "audiofile_reader.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#include "filemanager.h"

namespace audiofile {

static uint8_t filebuf[FILE_BUFSIZE];
static int file = -1;
static unsigned long nextBufferByte = 0;

static int find_sync(uint8_t *buf, int len) {
    for (int i = 0; i < len - 1; ++i) {
        // if (buf[i] == 0xff && (buf[i + 1] & 0xe0) == 0xe0) {
        if (buf[i] == 0xff && buf[i + 1] == 0xfb) {
            return i;
        }
    }
    return -1;
}

int open(const char *path) {
    file = filemanager::openFile(path);
    if (file < 0) {
        // std::cout << "bad file\n";
        return -1;
    }
    return 0;
}

void close() {
    nextBufferByte = 0;
    filemanager::closeFile(file);
}

int readNextBuffer() {
    int sync_start = 0, sync_end = 0;

    // std::cout << nextBufferByte << " now at this pos\n";
    filemanager::seek(file, nextBufferByte);

    if (filemanager::readFileToBuffer(file, filebuf, FILE_BUFSIZE) !=
        FILE_BUFSIZE) {
        puts("read fail");
        if (filemanager::eof(file)) {
            puts("eof");
        }
        return -1;
    }

    if (find_sync(filebuf, 2) == -1) {
        puts("not aligned! ");
        sync_start = find_sync(filebuf, FILE_BUFSIZE);
        while (sync_start == -1) {
            if (filemanager::readFileToBuffer(file, filebuf, FILE_BUFSIZE) !=
                FILE_BUFSIZE) {
                puts("read fail\n");
                if (filemanager::eof(file)) {
                    puts("eof\n");
                }
                return -1;
            };
            sync_start = find_sync(filebuf, FILE_BUFSIZE);
        }
        printf("start offset %i\n", sync_start);
        nextBufferByte += sync_start;
        memmove(filebuf, filebuf + sync_start, FILE_BUFSIZE - sync_start);

        if (filemanager::readFileToBuffer(file,
                                          filebuf + (FILE_BUFSIZE - sync_start),
                                          sync_start) != sync_start)
            return -1;
    }

    return 0;
}

void setUsedBytes(int n) { nextBufferByte += n; }

uint8_t *getBuffer() { return filebuf; }

} // namespace audiofile
#include "playlistfile.h"

#include <cstdio>
#include <string.h>
#include <string>

#include "filemanager.h"
#include "queue.h"

namespace playlistfile {

// TODO test 10, 1024
int FILE_BUFSIZE = 1024;

bool add_to_queue(const char *path) {
    printf("called\n");
    int file = filemanager::openFile(path);
    if (file < 0) {
        // bad file
        return true;
    }
    char filebuf[FILE_BUFSIZE];

    // clear current queue?
    // clear_queue()

    unsigned int leftover_bytes = 0;

    while (true) {
        printf("outer loop\n");
        filemanager::readFileToBuffer(file, filebuf + leftover_bytes,
                                      FILE_BUFSIZE - leftover_bytes);

        char *nptr, *ptr = filebuf;
        while (nptr = strchr(ptr, '\n')) {
            nptr[0] = '\0';
            add_to_queue_end(ptr);
            ptr = nptr + 1;
        }

        if (filemanager::eof(file)) {
            printf("breaking\n");
            break;
        } else {
            leftover_bytes = strlen(ptr);
            printf("leftover_bytes: %d\n", leftover_bytes);
            memcpy(filebuf, ptr, leftover_bytes);
        }
    }

    filemanager::closeFile(file);
    return false;
}

bool FUCKadd_to_queue(const char *path) {
    printf("strriten\n");
    int file = filemanager::openFile(path);
    if (file < 0) {
        // bad file
        return true;
    }

    char filebuf[FILE_BUFSIZE];

    // clear current queue?
    // clear_queue()

    unsigned int leftover_bytes = 0;
    while (true) {
        printf("loop\n");
        filemanager::readFileToBuffer(file, filebuf + leftover_bytes,
                                      FILE_BUFSIZE - leftover_bytes);
        char *nptr, *ptr = filebuf;

        // add files to queue
        // TODO this while loop is ignored and the function is
        // looped?????????????????????
        while (nptr = strchr(ptr, '\n')) {
            nptr[0] = '\0';
            printf("%s\n", ptr);
            add_to_queue(ptr);
            ptr = nptr + 1;
        }
        return false;
        // stich leftovers back together if buffer buffer wasn't big enough
        leftover_bytes = strlen(ptr);
        memcpy(filebuf, ptr, leftover_bytes);

        if (filemanager::eof(file))
            break;
    }
    filemanager::closeFile(file);
    return false;
}
} // namespace playlistfile

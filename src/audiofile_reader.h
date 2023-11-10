#pragma once

#include <cstdint>

#include "config.h"

const int FILE_BUFSIZE = 1024;

namespace audiofile {
int open(const char* path);
void close();
int readNextBuffer();
void setUsedBytes(int n);
uint8_t* getBuffer();
}

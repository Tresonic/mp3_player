#pragma once

#include "util.h"

namespace filemanager {
RET_TYPE list_dir(const char* path, char* files, int files_len, char* dirs, int dirs_len);

void initSd();
void deinitSd();
int openFile(const char* path);
void closeFile(int handle);
unsigned int readFileToBuffer(int handle, void* buffer, int numBytes);
}

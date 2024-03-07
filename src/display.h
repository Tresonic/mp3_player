#pragma once

#include "utils.h"


namespace display {
struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    int buflen;
};

RET_TYPE init(const int scl, const int sda);
void print(int x, int y, const char *str);
void printChar(int x, int y, const char c);
void display();

}

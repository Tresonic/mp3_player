#pragma once

#define min(a, b) a > b ? b : a

namespace config {
    const int PIN_I2S_CLK_BASE = 18;
    const int PIN_I2S_DATA = 20;

    const int FILE_BUF_SIZE = 2048;

    const int AUDIOBUFFER_SIZE = 4096;
    const int AUDIOBUFFER_NUM = 6;

    const int MAX_OPEN_FILES = 5;
}

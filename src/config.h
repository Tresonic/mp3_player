#pragma once

#include <cstdint>

#define min(a, b) a > b ? b : a

namespace config {
const int VOLUME_STEP = 3;

const int PIN_I2S_CLK_BASE = 26;
const int PIN_I2S_DATA = 28;

const int PIN_ENCODER_A = 10;
const int PIN_ENCODER_B = 11;

const int PIN_BTN_PAUSE = 21;

const int FILE_BUF_SIZE = 2048;

const int AUDIOBUFFER_SIZE = 4096;
const int AUDIOBUFFER_NUM = 6;

const int MAX_OPEN_FILES = 5;
}

#pragma once

#include <cstdint>

namespace player {
const int SAMPLE_BUF_NUM = 2;
const unsigned AUDIO_BUFSIZE = 2304;

void init();
void setVol(uint8_t);
uint8_t getVol();

void tick();
void play(const char *file);
void togglePause();
void stop();
bool isFinished();

int16_t *getLastFilledBuffer();
void usedCurrentBuffer();
} // namespace player
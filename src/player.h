#pragma once

#include <cstdint>

namespace player {
void init();
void setVol(uint8_t);
uint8_t getVol();
void tick();
void play(const char* file);
void togglePause();
void stop();
bool isFinished();
}
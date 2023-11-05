#pragma once

#include <cstdint>


namespace player {
void init();
void setVol(uint8_t);
void tick();
void play(const char* file);
void togglePause();
void stop();
bool isFinished();
}
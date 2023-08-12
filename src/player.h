#pragma once

namespace player {
void init();
void tick();
void play(const char* file);
void togglePause();
void stop();
bool isFinished();
}
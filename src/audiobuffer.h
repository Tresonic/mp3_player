#pragma once

#include <stdint.h>

namespace audiobuffer {
void reset();
int getNumWritableSamples();
int writeSamples(int16_t* src, int maxSamples, bool mono);
int16_t* getNextDmaBlock();
}

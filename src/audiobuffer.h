#pragma once

#include <cstdint>
#include <stdint.h>

namespace audiobuffer {
void reset();
int getNumWritableSamples();
int writeSamples(int16_t* src, int maxSamples, bool mono, uint8_t vol);
int16_t* getNextDmaBlock();
}

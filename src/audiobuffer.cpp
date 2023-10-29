#include "audiobuffer.h"

#include <stdio.h>

#include "pico/stdlib.h"

#include "config.h"

namespace audiobuffer {
using config::AUDIOBUFFER_NUM;
using config::AUDIOBUFFER_SIZE;

static int16_t mBuffer[AUDIOBUFFER_NUM * AUDIOBUFFER_SIZE] = { 0 };
static int mWrIdx = 0;
static int mDmaLockedBuffer = 1;

void reset()
{
    mWrIdx = 0;
    mDmaLockedBuffer = 1;
    // memset();
}

int getNumWritableSamples()
{
    int dmaIdx = mDmaLockedBuffer * AUDIOBUFFER_SIZE;
    if (dmaIdx < mWrIdx) {
        return count_of(mBuffer) - mWrIdx + dmaIdx;
    }
    return dmaIdx - mWrIdx;
}

inline static int16_t vol_scale(int16_t s, uint8_t v) {
    return static_cast<int16_t>(static_cast<int32_t>(s) * v >> 8);
}

int writeSamples(int16_t* src, int maxSamples, bool mono, uint8_t vol)
{
    int n = min(maxSamples, getNumWritableSamples());
    if (!mono) {
        for (int i = 0; i < n; ++i) {
            // mBuffer[mWrIdx++] = ((int)src[i])*vol >> 8;
            mBuffer[mWrIdx++] = vol_scale(src[i], vol);;
            // if (src[i])
            //     printf("%i\n", (int)src[i]);
            if (mWrIdx >= count_of(mBuffer)) {
                mWrIdx = 0;
            }
        }
    } else {
        for (int i = 0; i < n; ++i) {
            mBuffer[mWrIdx++] = vol_scale(src[i], vol);
            if (mWrIdx >= count_of(mBuffer)) {
                mWrIdx = 0;
            }
            mBuffer[mWrIdx++] = vol_scale(src[i], vol);;
            if (mWrIdx >= count_of(mBuffer)) {
                mWrIdx = 0;
            }
        }
    }
    return n;
}

int16_t* getNextDmaBlock()
{
    int newLockedBuffer = mDmaLockedBuffer + 1;
    if (newLockedBuffer >= AUDIOBUFFER_NUM) {
        newLockedBuffer = 0;
    }

    if (mWrIdx >= newLockedBuffer * AUDIOBUFFER_SIZE && mWrIdx <= (newLockedBuffer + 1) * AUDIOBUFFER_SIZE) {
        puts("dma overtook write!!!");
        return NULL;
    }
    // printf("new dma block: %i\n", newLockedBuffer);
    mDmaLockedBuffer = newLockedBuffer;
    int idx = mDmaLockedBuffer > 0 ? mDmaLockedBuffer * AUDIOBUFFER_SIZE - 1 : 0;
    return &mBuffer[idx];
}

}
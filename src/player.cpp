#include "player.h"

#include "hardware/timer.h"
#include <cstdint>

#include "mad.h"

#include "audiofile_reader.h"
#include "config.h"
#include "filemanager.h"
#include "i2s_dac.h"

namespace player {

static int16_t mSampleBuffer[SAMPLE_BUF_NUM][AUDIO_BUFSIZE];
static int mBufferIdx = 0;
static int mBufferIdxOld = 1;

static bool playing = false;
static bool finished = false;

static struct mad_stream stream;
static struct mad_frame frame;
static struct mad_synth synth;
static int mSampleRate = 48000;
static uint8_t vol = 128;
static unsigned bitrate;
static unsigned long bitrate_change_counter;

/// Scales the sample from internal MAD format to int16
inline static int16_t scale(mad_fixed_t sample) {
    /* round */
    if (sample >= MAD_F_ONE)
        return (INT16_MAX);
    if (sample <= -MAD_F_ONE)
        return (INT16_MIN);

    /* Conversion. */
    sample = sample >> (MAD_F_FRACBITS - 15);

    /* Volume scaling */
    return static_cast<int16_t>(static_cast<int32_t>(sample) * vol >> 8);
}

void init() {
    init_pio(config::PIN_I2S_CLK_BASE, config::PIN_I2S_DATA);
    mad_stream_init(&stream);
    mad_frame_init(&frame);
    mad_synth_init(&synth);
}

void setVol(uint8_t v) {
    printf("setting vol: %i\n", (int)v);
    vol = v;
}

uint8_t getVol() { return vol; }

int getSampleRate() { return mSampleRate; }

int decodeNextFrame() {
    if (mBufferIdx >= SAMPLE_BUF_NUM)
        mBufferIdx = 0;

    int err = audiofile::readNextBuffer();
    uint8_t *buf_ptr = audiofile::getBuffer();

    mad_stream_buffer(&stream, buf_ptr, FILE_BUFSIZE);
    int rc = mad_frame_decode(&frame, &stream);
    if (rc == 0) {
        // std::cout << stream.next_frame - filebuf << " offset\n";
        audiofile::setUsedBytes(stream.next_frame - buf_ptr);
        // std::cout << "decoded!!\n";
        mad_synth_frame(&synth, &frame);

        if (synth.pcm.length > 0) {
            if (synth.pcm.samplerate != mSampleRate) {
                mSampleRate = synth.pcm.samplerate;
                set_pio_frequency(mSampleRate);
            }
            // printf("got audio: %i samples; samplerate: %i\n",
            // (int)synth.pcm.length, (int)mSampleRate);
            int i = 0;

            if (synth.pcm.channels == 2) {
                for (int j = 0; j < synth.pcm.length; j++) {
                    mSampleBuffer[mBufferIdx][i++] =
                        scale(synth.pcm.samples[0][j]);
                    mSampleBuffer[mBufferIdx][i++] =
                        scale(synth.pcm.samples[1][j]);
                }
            } else if (synth.pcm.channels == 1) {
                for (int j = 0; j < synth.pcm.length; j++) {
                    mSampleBuffer[mBufferIdx][i++] =
                        scale(synth.pcm.samples[0][j]);
                    mSampleBuffer[mBufferIdx][i++] =
                        scale(synth.pcm.samples[0][j]);
                }
            } else {
                printf("unsupported channel number: %i\n",
                       (int)synth.pcm.channels);
            }
        }
    } else {
        printf("%i decode failed!\n", rc);
        return -1;
    }
    return 1;
}

unsigned calcAvgBitrate(unsigned cur_bitrate, unsigned new_bitrate,
                        unsigned long counter) {
    // maybe implement a calc stop after a few sec, as the bitrate probably
    // won't change that much anymore
    if (cur_bitrate == new_bitrate || counter == 0) {
        return new_bitrate;
    } else {
        return ((((unsigned long long)cur_bitrate * counter) + new_bitrate) /
                (counter + 1));
    }
}

void tick() {
    if (mBufferIdx != mBufferIdxOld) {
        mBufferIdxOld = mBufferIdx;
        if (decodeNextFrame() == -1) {
            stop();
        }

        bitrate = calcAvgBitrate(bitrate, getBitrate(), bitrate_change_counter);
        bitrate_change_counter++;
        printf("bitrate: %d\n", getBitrate());
        printf("%lu: %d\n", bitrate_change_counter, bitrate);
    }
}

void usedCurrentBuffer() {
    mBufferIdx++;
    if (mBufferIdx >= SAMPLE_BUF_NUM) {
        mBufferIdx = 0;
    }
}

int16_t *getLastFilledBuffer() { return mSampleBuffer[mBufferIdx]; }

int getLastFilledBufferIdx() { return mBufferIdx; }

void play(const char *file) {
    if (!finished) {
        stop();
    }
    mBufferIdx = 0;
    mBufferIdxOld = 1;
    if (audiofile::open(file) == -1) {
        puts("could not open file");
        return;
    }
    decodeNextFrame();
    i2s_dac_set_enabled(true);
    playing = true;
    finished = false;
    bitrate_change_counter = 0;
}

void togglePause() {
    if (!playing) {
        // playStart = true;
        playing = true;
    } else {
        playing = false;
    }
    i2s_dac_set_enabled(playing);
}

void stop() {
    audiofile::close();
    i2s_dac_set_enabled(false);
    playing = false;
    finished = true;
}

bool isPlaying() { return playing; }
bool isFinished() { return finished; }

int getBitrate() { return frame.header.bitrate / 1000; }

} // namespace player

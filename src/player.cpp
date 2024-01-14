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

static bool playing = true;
static bool finished = false;

static struct mad_stream stream;
static struct mad_frame frame;
static struct mad_synth synth;
static int mSampleRate = 48000;
static uint8_t vol = 128;
static unsigned long long bitrate;
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

int getBitrate() { return frame.header.bitrate / 1000; }

void tick() {
    if (mBufferIdx != mBufferIdxOld) {
        mBufferIdxOld = mBufferIdx;
        if (decodeNextFrame() == -1) {
            stop();
        }

        bitrate = (bitrate_change_counter == 0) ? getBitrate()
                                                : bitrate + getBitrate();
        bitrate_change_counter++;
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

void enforcePlaying() {
    // dont force playing if there is no song to play
    if (!finished)
        i2s_dac_set_enabled(playing);
}

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
    finished = false;
    bitrate_change_counter = 0;
    enforcePlaying();
}

void togglePause() {
    // this will work if queue is finished. could be disabled by same check as
    // in enforcePlaying
    if (!playing) {
        playing = true;
    } else {
        playing = false;
    }
    enforcePlaying();
}

void stop() {
    // TODO should probably clear current dac buffer to avoid short playing of
    // last song when doing pause -> change song -> play
    audiofile::close();
    i2s_dac_set_enabled(false);
    finished = true;
}

bool isPlaying() { return playing; }
bool isFinished() { return finished; }

float secToMin(unsigned sec) { return sec / 60 + (sec % 60) / (float)100; }

unsigned getLength() {
    return (audiofile::getSize() * 0.008) /
           (float)(bitrate / bitrate_change_counter);
}

} // namespace player

#pragma once

#include "MP3DecoderMAD.h"
#include "audiobuffer.h"
#include "config.h"
#include "filemanager.h"
#include "i2s_dac.h"

#include "bbng.h"

using namespace libmad;

class Player {
public:
    Player()
        : mp3(pcmDataCallback)
    {
    }

    void init()
    {
        init_pio(config::PIN_I2S_CLK_BASE, config::PIN_I2S_DATA);
    }

    void tick()
    {
        if (!mPlaying)
            return;
        int w = audiobuffer.getNumWritableSamples();
        // printf("availableSamples %i\n", w);
        if (w < config::FILE_BUF_SIZE * 8) {
            return;
        }
        absolute_time_t bef = get_absolute_time();
        int readBytes = filemanager.readFileToBuffer(mFp, mFilebuffer, config::FILE_BUF_SIZE);
        int diff = absolute_time_diff_us(bef, get_absolute_time()) / 1000;
        printf("bytes read: %u; read time: %i\n", readBytes, diff);

        // mp3.write(mFilebuffer, readBytes);
        // if (readBytes < sizeof(mFilebuffer))
        //     stop();

        sleep_ms(15);
        static int idx = 0;
        mp3.write(&bbng_mp3[idx], config::FILE_BUF_SIZE);
        idx += config::FILE_BUF_SIZE;
        if(idx > sizeof(bbng_mp3))
            stop();

    }

    void play(const char* file)
    {
        mFp = filemanager.openFile(file);
        if (mFp < 0)
            return;
        mp3.begin();
        i2s_dac_set_enabled(true);
        mPlaying = true;
        mFinished = false;
    }

    void togglePause() {
        mPlaying = !mPlaying;
        i2s_dac_set_enabled(mPlaying);
    }

    void stop()
    {
        i2s_dac_set_enabled(false);
        mp3.end();
        mPlaying = false;
        mFinished = true;
    }

    bool isPlaying() { return mPlaying; }
    bool isFinished() { return mFinished; }

    // int16_t* getNextDmaBlock() { return audiobuffer.getNextDmaBlock(); }

private:
    static void pcmDataCallback(MadAudioInfo& info, int16_t* pcm_buffer, size_t len)
    {
        // printf("PCM Data: %i Hz, %i Channels, %zu Samples\n", info.sample_rate,
        //    info.channels, len);
        int s = audiobuffer.getNumWritableSamples();
        if (s < len) {
            printf("not enough space in audiobuffer: %i\n", s);
        }
        audiobuffer.writeSamples(pcm_buffer, len);
    }

    MP3DecoderMAD mp3;
    uint8_t mFilebuffer[config::FILE_BUF_SIZE];
    bool mPlaying;
    bool mFinished;
    int mFp;
};

static Player player;
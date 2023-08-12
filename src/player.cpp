#include "player.h"

#include "hardware/timer.h"

#include "MP3DecoderMAD.h"
#include "audiobuffer.h"
#include "config.h"
#include "filemanager.h"
#include "i2s_dac.h"

namespace player {

using namespace libmad;

static void pcmDataCallback(MadAudioInfo& info, int16_t* pcm_buffer, size_t len);

static MP3DecoderMAD mp3(pcmDataCallback);
static uint8_t mFilebuffer[config::FILE_BUF_SIZE];
static bool mPlaying;
static bool mFinished;
static int mFp;
static bool playStart = false;

void init()
{
    init_pio(config::PIN_I2S_CLK_BASE, config::PIN_I2S_DATA);
}

void tick()
{
    const int kbps = 128;
    const int readSize = kbps * 4;
    const float buffersPerSec = kbps / 8 * 1000.f / (float)readSize;
    const uint32_t usPerBuffer = 1000000.f / buffersPerSec;
    static uint32_t lastDecode;
    uint32_t now = time_us_32();

    bool noSpace = audiobuffer::getNumWritableSamples() < config::AUDIOBUFFER_NUM * config::AUDIOBUFFER_SIZE / 2;
    bool plentySpace = audiobuffer::getNumWritableSamples() > (config::AUDIOBUFFER_NUM - 1) * config::AUDIOBUFFER_SIZE;

    if ((!mPlaying || (now - lastDecode <= usPerBuffer) || noSpace) && !playStart)
        return;

    lastDecode = now;

    uint32_t bef = time_us_32();
    int readBytes = filemanager::readFileToBuffer(mFp, mFilebuffer, readSize);
    int diff = (time_us_32() - bef) / 1000;
    printf("bytes read: %u; read time: %i\n", readBytes, diff);

    mp3.write(mFilebuffer, readBytes);
    if (readBytes < readSize)
        stop();

    // if (!mPlaying)
    //     return;
    // int w = audiobuffer.getNumWritableSamples();
    // // printf("availableSamples %i\n", w);
    // if (w < config::FILE_BUF_SIZE * 8) {
    //     return;
    // }
    // absolute_time_t bef = get_absolute_time();
    // int readBytes = filemanager.readFileToBuffer(mFp, mFilebuffer, config::FILE_BUF_SIZE);
    // int diff = absolute_time_diff_us(bef, get_absolute_time()) / 1000;
    // printf("bytes read: %u; read time: %i\n", readBytes, diff);

    // sleep_ms(12);

    // mp3.write(mFilebuffer, readBytes);
    // if (readBytes < sizeof(mFilebuffer))
    //     stop();

    // sleep_ms(15);
    // static int idx = 0;
    // mp3.write(&bbng_mp3[idx], config::FILE_BUF_SIZE);
    // idx += config::FILE_BUF_SIZE;
    // if(idx > sizeof(bbng_mp3))
    //     stop();
}

void play(const char* file)
{
    mFp = filemanager::openFile(file);
    if (mFp < 0)
        return;
    mp3.begin();
    playStart = true;
    mPlaying = true;
    mFinished = false;
}

void togglePause()
{
    if (!mPlaying) {
        playStart = true;
        mPlaying = true;
    } else {
        mPlaying = false;
        i2s_dac_set_enabled(false);
    }
}

void stop()
{
    filemanager::closeFile(mFp);
    i2s_dac_set_enabled(false);
    mp3.end();
    mPlaying = false;
    mFinished = true;
}

bool isPlaying() { return mPlaying; }
bool isFinished() { return mFinished; }

static void pcmDataCallback(MadAudioInfo& info, int16_t* pcm_buffer, size_t len)
{
    printf("PCM Data: %i Hz, %i Channels, %zu Samples\n", info.sample_rate,
        info.channels, len);

    static int samplerate = 48000;
    if (info.sample_rate != samplerate) {
        samplerate = info.sample_rate;
        set_pio_frequency(samplerate);
    }

    if (playStart) {
        playStart = false;
        i2s_dac_set_enabled(true);
    }

    int s = audiobuffer::getNumWritableSamples();
    if (s < len) {
        printf("not enough space in audiobuffer: %i\n", s);
    }
    audiobuffer::writeSamples(pcm_buffer, len, info.channels == 1);
}

}